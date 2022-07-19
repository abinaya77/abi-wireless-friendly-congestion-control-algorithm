
///in paper ss *by 1,,,, in diff havent multiplied wid one way delay....
#include "tcp-wfcc.h"
#include "tcp-socket-base.h"
#include "ns3/log.h"
#include "tcp-ledbat.h"
#include <vector>
#include "ns3/tcp-congestion-ops.h"
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpWFCC");

NS_OBJECT_ENSURE_REGISTERED (TcpWFCC);

TypeId
TcpWFCC::GetTypeId (void)

{
  static TypeId tid = TypeId ("ns3::TcpWFCC")
    .SetParent<TcpNewReno> ()
    .AddConstructor<TcpWFCC> ()
    .SetGroupName ("Internet")

  

;
  return tid;
}




TcpWFCC::TcpWFCC (void)
: TcpNewReno (),
m_cWnd(),
m_wgf(0),
m_cntRtt (0),
m_minRtt (Time::Max ()),
m_maxRtt (Time::Min ()),
m_begSndNxt (0),
m_avgRtt (Time::Max ()),
m_lastRtt(Time::Max ())
    
                   

{ NS_LOG_FUNCTION (this);
  InitCircBuf (m_noiseFilter);
}

void TcpWFCC::InitCircBuf (struct OwdCircBuf &buffer)
{
  NS_LOG_FUNCTION (this);
  buffer.buffer.clear ();
  buffer.min = 0;
}

TcpWFCC::TcpWFCC (const TcpWFCC& sock)
  : TcpNewReno (sock),
m_cWnd(sock.m_cWnd),
m_wgf (sock.m_wgf),
m_cntRtt (sock.m_cntRtt),
m_minRtt (sock.m_minRtt),
m_maxRtt (sock.m_maxRtt),
m_begSndNxt (0),
m_avgRtt (sock.m_avgRtt),
m_lastRtt(sock.m_lastRtt)

{
  NS_LOG_FUNCTION (this);
 m_noiseFilter = sock.m_noiseFilter;
}


TcpWFCC::~TcpWFCC (void)
{
}


uint32_t TcpWFCC::CurrentDelay (FilterFunction filter)
{
  NS_LOG_FUNCTION (this);
  return filter (m_noiseFilter);
}

uint32_t TcpWFCC::MinCircBuf (struct OwdCircBuf &b)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (b.buffer.size () == 0)
    {
      return ~0U;
    }
  else
    {
      return b.buffer[b.min];
    }
}






void
TcpWFCC::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t m_cWnd,
                     const Time& rtt)
{
  NS_LOG_FUNCTION (this << tcb << m_cWnd << rtt);

  if (rtt.IsZero ())
    {
      return;
    }

  m_minRtt =  (std::min (m_minRtt, rtt));
  NS_LOG_DEBUG ("Updated m_minRtt = " << m_minRtt);

 /* m_baseRtt = std::min (m_baseRtt, rtt);
  NS_LOG_DEBUG ("Updated m_baseRtt = " << m_baseRtt);
*/
  m_maxRtt = std::max (rtt, m_maxRtt);
  NS_LOG_DEBUG ("Updated m_maxRtt = " << m_maxRtt);

  // Update RTT counter
  m_cntRtt++;
  NS_LOG_DEBUG ("Updated m_cntRtt = " << m_cntRtt);
}

void
TcpWFCC::EnableWFCC (Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this << tcb);
  m_cntRtt = 0;
  m_minRtt = Time::Max ();
}

void
TcpWFCC::DisableWFCC ()
{
  NS_LOG_FUNCTION (this);
}

void
TcpWFCC::CongestionStateSet (Ptr<TcpSocketState> tcb,
                              const TcpSocketState::TcpCongState_t newState)
{
  NS_LOG_FUNCTION (this << tcb << newState);
  if (newState == TcpSocketState::CA_OPEN)
    {
      EnableWFCC (tcb);
    }
  else
    {
      DisableWFCC();
    }
}


void
TcpWFCC::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

   
  if (tcb->m_lastAckedSeq >= m_begSndNxt)
    { // A WFCC cycle has finished, we do Vegas cwnd adjustment every RTT.

      NS_LOG_LOGIC ("A Vegas cycle has finished, we adjust cwnd once per RTT.");

      // Save the current right edge for next Vegas cycle
      m_begSndNxt = tcb->m_nextTxSequence;

      /*
       * We perform Vegas calculations only if we got enough RTT samples to
       * insure that at least 1 of those samples wasn't from a delayed ACK.
       */
      if (m_cntRtt <= 2)
        {  // We do not have enough RTT samples, so we should behave like Reno
          NS_LOG_LOGIC ("We do not have enough RTT samples to do Vegas, so we behave like NewReno.");
          TcpNewReno::IncreaseWindow (tcb, segmentsAcked);
        }

else
        {
          NS_LOG_LOGIC ("We have enough RTT samples to perform Vegas calculations");
	   
          uint32_t diff;
          uint32_t t;
          
uint64_t current_delay = CurrentDelay (&TcpWFCC::MinCircBuf);
         
          double tmp = m_maxRtt.GetSeconds ();
	double tct =m_minRtt.GetSeconds ();
          t = tct +(tmp -tct)*0.15;
          //NS_LOG_DEBUG ("Calculated targetCwnd = " << targetCwnd);
         // NS_ASSERT (segCwnd >= targetCwnd); // implies baseRtt <= minRtt

        
          diff = (1-0.125) + (0.125*current_delay);    ////////////////////havent multiplied one way delay
          NS_LOG_DEBUG ("Calculated diff = " << diff);

          if (diff <  t && (tcb->m_cWnd < tcb->m_ssThresh))
            {
              
		tcb->m_cWnd =tcb->m_cWnd + 1;              
            
            
              tcb->m_ssThresh = GetSsThresh (tcb, 0);
              NS_LOG_DEBUG ("Updated cwnd = " << tcb->m_cWnd <<
                            " ssthresh=" << tcb->m_ssThresh);
            }
          else if (tcb->m_cWnd < tcb->m_ssThresh)
            {     // Slow start mode
              NS_LOG_LOGIC ("We are in slow start and diff < m_gamma, so we "
                            "follow NewReno slow start");
              TcpNewReno::SlowStart (tcb, segmentsAcked);
            }


else 
{

     
	tcb->m_cWnd =tcb->m_cWnd/2;
	tcb->m_cWnd = tcb->m_cWnd+1;
	NS_LOG_INFO ("In reduction, updated to cwnd " << tcb->m_cWnd );
	
    
}
 
  tcb->m_ssThresh = std::max (tcb->m_ssThresh, 3 * tcb->m_cWnd / 4);
          NS_LOG_DEBUG ("Updated ssThresh = " << tcb->m_ssThresh);
        }

      // Reset cntRtt & minRtt every RTT
      m_cntRtt = 0;
      m_minRtt = Time::Max ();
    }
  else if (tcb->m_cWnd < tcb->m_ssThresh)
    {
      TcpNewReno::SlowStart (tcb, segmentsAcked);
    }
}

uint32_t
TcpWFCC::GetSsThresh (Ptr<const TcpSocketState> tcb,
                       uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << tcb << bytesInFlight);
  return ( std::max (2 * tcb->m_segmentSize, bytesInFlight / 2));
}

} 
// namespace ns3
