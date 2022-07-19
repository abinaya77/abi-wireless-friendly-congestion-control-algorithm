#ifndef TCPWFCC_H
#define TCPWFCC_H

#include "ns3/tcp-congestion-ops.h"
#include "tcp-socket-base.h"

#include "tcp-ledbat.h"
#include <vector>
#include "ns3/tcp-congestion-ops.h"


namespace ns3 {

class TcpWFCC : public TcpNewReno
{

public: 
  enum State : uint32_t
  {
    TcpWFCC_VALID_OWD  = (1 << 1),  //!< If valid timestamps are present
    TcpWFCC_CAN_SS     = (1 << 3)   //!< If LEDBAT allows Slow Start
  };

 struct OwdCircBuf
  {
    std::vector<uint32_t> buffer; //!< Vector to store the delay
    uint32_t min;  //!< The index of minimum value
  };
  void InitCircBuf (struct OwdCircBuf &buffer);

  /// Filter function used by LEDBAT for current delay
  typedef uint32_t (*FilterFunction)(struct OwdCircBuf &);

  /**
   * \brief Return the minimum delay of the buffer
   *
   * \param b The buffer
   * \return The minimum delay
   */
  static uint32_t MinCircBuf (struct OwdCircBuf &b);

  /**
   * \brief Return the value of current delay
   *
   * \param filter The filter function
   * \return The current delay
   */
  uint32_t CurrentDelay (FilterFunction filter);



 
  static TypeId GetTypeId (void);

  TcpWFCC (void);

  TcpWFCC (const TcpWFCC& sock);
  virtual ~TcpWFCC (void);

  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                          const Time& rtt);


  virtual void CongestionStateSet (Ptr<TcpSocketState> tcb,
                                   const TcpSocketState::TcpCongState_t newState);

  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);



public:
 
  void EnableWFCC (Ptr<TcpSocketState> tcb);

  void DisableWFCC ();

public:
 
 uint32_t m_cWnd;
 uint32_t m_wgf;
uint32_t m_cntRtt; 
Time m_minRtt;   
Time m_maxRtt;     
SequenceNumber32 m_begSndNxt;                   
Time m_avgRtt; 
Time m_lastRtt;   
  OwdCircBuf m_noiseFilter; 
    
                   
};

} // namespace ns3

#endif // TCPWFCC_H
