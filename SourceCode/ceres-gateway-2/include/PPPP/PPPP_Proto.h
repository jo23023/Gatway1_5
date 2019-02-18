#ifndef __INCLUDE_FILE_PPPP_PROTOCOL
#define __INCLUDE_FILE_PPPP_PROTOCOL

#include "PPPP_Type.h"
#define PPPP_PROTO_MAX_PACKET_SIZE 1280 
//// P2P Service Port
#define PPPP_P2P_SERVER_PORT	32100
//// Max Number of Relay replied when receving MSG_LIST_REQ
#define PPPP_MAX_LIST_RELAY		8

//// Part A: Session Protocol
typedef enum {	
//// Hello Message
	MSG_HELLO = 0x00,
	MSG_HELLO_ACK = 0x01,
	MSG_HELLO_TO = 0x02,
	MSG_HELLO_TO_ACK = 0x03,

//// QueryDID
	MSG_QUERY_DID = 0x08,
	MSG_QUERY_DID_ACK = 0x09,

//// Device Login
	MSG_DEV_LGN = 0x10,
	MSG_DEV_LGN_ACK = 0x11,
	MSG_DEV_LGN_CRC = 0x12,   //// ver 1.2
	MSG_DEV_LGN_ACK_CRC = 0x13,  //// ver 1.2

//// P2P Request
	MSG_P2P_REQ = 0x20,
	MSG_P2P_REQ_ACK = 0x21,

//// LAN Search
	MSG_LAN_SEARCH=0x30,   //// C ---> D, D reply MSG_PUNCH_TO

//// Hole Punching action ...
	MSG_PUNCH_TO=0x40,    //// S --> C/D
	MSG_PUNCH_PKT=0x41,  //// C<--->D, 
	MSG_P2P_RDY=0x42,  //// D --> C, means P2P is ready
	////	if client get MSG_PUNCH_PKT, it send MSG_PUNCH_PKT back to device.
	////	if device get MSG_PUNCH_PKT, send MSG_P2P_RDY to client.


//// Relay Server Login, R<--> S
	MSG_RS_LGN = 0x60,
	MSG_RS_LGN_ACK = 0x61,	
	
//// Relay Server List Request, C<--> S
	MSG_LIST_REQ1 = 0x67,		//// Also ask for super device and relay server
	MSG_LIST_REQ = 0x68,
	MSG_LIST_REQ_ACK = 0x69,

//// Relay Server Choosing
	MSG_RLY_HELLO=0x70, 
	MSG_RLY_HELLO_ACK=0x71,    //// Choose the best response one, IP
	MSG_RLY_PORT=0x72, 
	MSG_RLY_PORT_ACK=0x73,    //// Get Relay service IP:Port

//// Relay statics
	MSG_RLY_BYTE_COUNT=0x78, 
	
//// RLY action...
	MSG_RLY_REQ=0x80, 
	MSG_RLY_REQ_ACK=0x81, 	
	MSG_RLY_TO=0x82,    //// S --> C/D
	MSG_RLY_PKT=0x83,
	MSG_RLY_RDY=0x84,

//// Super Device 
	MSG_SDEV_RUN = 0x90,  //// Super Device' Relay service start Message
	MSG_SDEV_LGN = 0x91,  //// Super Device Login
	MSG_SDEV_LGN_ACK = 0x91,  //// Super Device Login ACK

	MSG_SDEV_LGN_CRC = 0x92,  //// Super Device Login , ver 1.2
	MSG_SDEV_LGN_ACK_CRC = 0x92,  //// Super Device Login ACK , ver 1.2


//// DRW Packet
	MSG_DRW=0xD0, 
	MSG_DRW_ACK=0xD1, 
//// Session Alive
	MSG_ALIVE=0xE0,
	MSG_ALIVE_ACK=0xE1,
//// Session Close
	MSG_CLOSE=0xF0,


//// For Management Function
	MSG_MGM_DUMP_LOGIN_DID=0xF4,
	MSG_MGM_DUMP_LOGIN_DID_DETAIL=0xF5,
	MSG_MGM_DUMP_LOGIN_DID_1=0xF6,
	MSG_MGM_LOG_CONTROL=0xF7,
	MSG_MGM_REMOTE_MANAGEMENT=0xF8
}ENUM_PPPP_SESSION_MESSAGE;

//// The PPPP Session Protocol Header
typedef struct{	
	UCHAR Magic_Version;  ////  Magic: 0xF , Version: 0x1  --> Magic_Version = 0xF1
	UCHAR Type;	   //// ENUM_AV_HEADER_TYPE
	UINT16 Size;    //// Size of following data, 0 ~ 65535
} st_PPPP_SessionHeader;


//// MSG_HELLO (No struct), MSG_HELLO_ACK
typedef struct{	
	struct sockaddr_in WanAddr;
} st_PPPP_HelloAck;

//// 	MSG_HELLO_TO, MSG_HELLO_TO_ACK  (No struct)
typedef struct{	
	struct sockaddr_in TargetAddr1;
	struct sockaddr_in TargetAddr2;
} st_PPPP_HelloTo;


//// MSG_DEV_LGN, MSG_DEV_LGN_ACK
typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
	CHAR NATType;
	UCHAR APIVersion[3];
	struct sockaddr_in LocalAddr;
} st_PPPP_DevLgn;

typedef struct{
	CHAR Result;  //// 0: Login Successfully, -1: CheckCode Mismatch, -2: Out of Date, -3: Invalid Prefix
	CHAR Reserved[3];
} st_PPPP_DevLgnAck;

//// MSG_DEV_LGN_CRC, MSG_DEV_LGN_ACK_CRC  //// ver 1.2
typedef struct{
	st_PPPP_DevLgn DevLgn;
	UCHAR CRC[4];
} st_PPPP_DevLgn_CRC;

typedef struct{
	st_PPPP_DevLgnAck DevLgnAck;
	UCHAR CRC[4];
} st_PPPP_DevLgnAck_CRC;


//// MSG_P2P_REQ, MSG_P2P_REQ_ACK 
typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
	struct sockaddr_in LocalAddr;
} st_PPPP_P2PReq;

typedef struct{
	CHAR Result;  //// 0: Req Successfully, -1: CheckCode Mismatch, -2: Out of Date, -3: Invalid Prefix
	CHAR Reserved[3];
} st_PPPP_P2PReqAck;


//// MSG_LAN_SEARCH (No struct)

//// MSG_PUNCH_TO, MSG_PUNCH_PKT,  MSG_P2P_RDY
typedef struct{
	struct sockaddr_in ToAddr;
} st_PPPP_PunchTo;

typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
} st_PPPP_PunchPkt;

typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
} st_PPPP_P2PRdy;


//// MSG_RS_LGN ,  MSG_RS_LGN_ACK 
typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
	UINT32 BandWidth; // in unit of Mbps
	UINT32 UserNumber; // number of relay service
} st_PPPP_RSLgn;

typedef struct{
	CHAR Result;   //// 0: Login Successfully, -1: CheckCode Mismatch, -3: Invalid Prefix
	CHAR Reserved[3];
} st_PPPP_RSLgnAck;


//// MSG_LIST_REQ1
typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
} st_PPPP_ListReq1;

//// MSG_LIST_REQ (No struct), MSG_LIST_REQ_ACK
typedef struct{
	CHAR ListNumber;  // number of Relay server address followed
	CHAR Reserved[3];
} st_PPPP_ListReqAck; 


//// MSG_RLY_HELLO (No struct), MSG_RLY_HELLO_ACK (No struct)


//// MSG_RLY_PORT (No struct), MSG_RLY_PORT_ACK
typedef struct{
	UINT32 MagicWord;
	UINT16 Port;
	CHAR Reserved[2];
} st_PPPP_RlyPortAck;

//// MSG_RLY_BYTE_COUNT
typedef struct{
  	UINT32 Count;
} st_PPPP_RlyByteCount;


//// MSG_RLY_REQ, MSG_RLY_REQ_ACK
typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
	struct sockaddr_in RLYAddr;
	UINT32 MagicWord;
} st_PPPP_RlyReq;

typedef struct{
	CHAR Result;
	CHAR Reserved[3];
} st_PPPP_RlyReqAck;


//// MSG_RLY_TO, MSG_RLY_PKT, MSG_RLY_RDY(No struct)
typedef struct{
	struct sockaddr_in RlyToAddr;
	UINT32 MagicWord;
} st_PPPP_RlyTo;

typedef struct{
	UINT32 MagicWord;
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
	CHAR CorD;  //// CorD=0 --> Client, CorD=1--> Device
	CHAR Reserved[3];
} st_PPPP_RlyPkt;

typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
} st_PPPP_RlyRdy;


//// MSG_SDEV_RUN(No struct), MSG_SDEV_LGN, MSG_SDEV_LGN_ACK
typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
} st_PPPP_SDevLgn;

typedef struct{
	struct sockaddr_in WanAddr;
} st_PPPP_SDevLgnAck;


//// MSG_SDEV_RUN(No struct), MSG_SDEV_LGN_CRC, MSG_SDEV_LGN_ACK_CRC  //// ver 1.2
typedef struct{
	st_PPPP_SDevLgn SdevLgn;
	UCHAR CRC[4];
} st_PPPP_SDevLgn_CRC;

typedef struct{
	st_PPPP_SDevLgnAck SdevLgnAck;
	UCHAR CRC[4];
} st_PPPP_SDevLgnAck_CRC;


//// Part B: DRW Protocol
//// DRWPacket = st_PPPP_AVHeader + st_PPPP_DRWHeader + Data
//// DRWAckPacket = st_PPPP_AVHeader + st_PPPP_DRWAckHeader + Indexs

typedef struct{	
	UCHAR Magic_Version; 	 //// Magic: 0xD , Version: 0x1  --> Magic_Version = 0xD1
	UCHAR Channel;	//// The Channel ID
	UINT16 Index;    	//// Index in Linklist
} st_PPPP_DRWHeader;

typedef struct{	
	UCHAR Magic_Version; 	 //// Magic: 0xD , Version: 0x1  --> Magic_Version = 0xD1
	UCHAR Channel;	//// The Channel ID
	UINT16 NumberOfIndex;    	//// Number of Indexs followed
} st_PPPP_DRWAckHeader;

//// Part C: Management Protocol
//// MSG_MGM_DUMP_LOGIN_DID (No struct), MSG_MGM_DUMP_LOGIN_DID_DETAIL(No struct)
//// MSG_MGM_DUMP_LOGIN_DID_1, MSG_MGM_LOG_CONTROL, MSG_MGM_REMOTE_MANAGEMENT
typedef struct{
	CHAR Prefix[8];
	UINT32 SerialNumber;
	CHAR CheckCode[8];
} st_PPPP_MGMDumpLoginDID1;

typedef struct{
	CHAR Command;  // 0: Log OFF, 1: Log ON
	CHAR Reserved[3];
} st_PPPP_MGMLogControl;

typedef struct{
	UCHAR Magic_Version; //Magic: 0xF , Version: 0x1  --> Magic_Version = 0xF1
	UCHAR CMDorRESP;  // 0: Command, 1: Resp
	UCHAR FunctionID; 
	CHAR Result;  // 0: succeed, -1: Wrong Password  
	UCHAR PacketIndex;
	UCHAR PacketTotalNum;  // 0 means can't tell how many packets in total
	UINT16 DataSize;  // size of data in this packet
} st_PPPP_MGMRemoteManagement;

//// Function declare
void htonAddr(const struct sockaddr_in *src, struct sockaddr_in *result);
void ntohAddr(const struct sockaddr_in *src, struct sockaddr_in *result);

void PPPP_Proto_Write_Header(st_PPPP_SessionHeader* pH, UCHAR MsgType, UINT16 Size);
INT32 PPPP_Proto_Read_Header(st_PPPP_SessionHeader Header, UCHAR *MsgType, UINT16 *Size);
void PPPP_Proto_Write_DevLgn(st_PPPP_DevLgn *pDevLgn, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, CHAR NATType, UCHAR *APIVersion,	struct sockaddr_in *LocalAddr);
void PPPP_Proto_Read_DevLgn(st_PPPP_DevLgn *pDevLgn, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode, CHAR *NATType, UCHAR *APIVersion,	struct sockaddr_in *LocalAddr);
void PPPP_Proto_Write_P2PReq(st_PPPP_P2PReq *pP2PReq, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, struct sockaddr_in *LocalAddr);
void PPPP_Proto_Read_P2PReq(st_PPPP_P2PReq *pP2PReq, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode, struct sockaddr_in *LocalAddr);
void PPPP_Proto_Write_RSLgn(st_PPPP_RSLgn *pRSLgn, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, UINT32 BandWidth, UINT32 UserNumber);
void PPPP_Proto_Read_RSLgn(st_PPPP_RSLgn *pRSLgn, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode, UINT32 *BandWidth, UINT32 *UserNumber);
void PPPP_Proto_Write_RlyReq(st_PPPP_RlyReq *pRlyReq, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, struct sockaddr_in *RLYAddr, UINT32 MagicWord);
void PPPP_Proto_Read_RlyReq(st_PPPP_RlyReq *pRlyReq, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode, struct sockaddr_in *RLYAddr, UINT32 *MagicWord);
void PPPP_Proto_Write_RlyTo(st_PPPP_RlyTo *pRlyTo, struct sockaddr_in *RlyToAddr, UINT32 MagicWord);
void PPPP_Proto_Read_RlyTo(st_PPPP_RlyTo *pRlyTo, struct sockaddr_in *RlyToAddr, UINT32 *MagicWord);
void PPPP_Proto_Write_RlyPortAck(st_PPPP_RlyPortAck *pRlyPortAck, UINT32 MagicWord, UINT16 Port);
void PPPP_Proto_Read_RlyPortAck(st_PPPP_RlyPortAck *pRlyPortAck, UINT32 *MagicWord, UINT16 *Port);
void PPPP_Proto_Write_RlyPkt(st_PPPP_RlyPkt *pRlyPkt, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, CHAR CorD,UINT32 MagicWord);
void PPPP_Proto_Read_RlyPkt(st_PPPP_RlyPkt *pRlyPkt, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode, CHAR *CorD,UINT32 *MagicWord);
void PPPP_Proto_Write_PunchPkt(st_PPPP_PunchPkt *pPunchPkt, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Read_PunchPkt(st_PPPP_PunchPkt *pPunchPkt, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Write_P2PRdy(st_PPPP_P2PRdy *pP2PRdy, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Read_P2PRdy(st_PPPP_P2PRdy *pP2PRdy, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Write_RlyRdy(st_PPPP_RlyRdy *pRlyRdy, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Read_RlyRdy(st_PPPP_RlyRdy *pRlyRdy, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode);
void PPPP_DRW_Write_Header(st_PPPP_DRWHeader *pDRWHeader, UCHAR Channel, UINT16 Index);
INT32 PPPP_DRW_Read_Header(st_PPPP_DRWHeader* pDRWHeader, UCHAR *Channel, UINT16 *Index);
void PPPP_DRWAck_Write_Header(st_PPPP_DRWAckHeader *pDRWAckHeader, UCHAR Channel, UINT16 NumberOfIndex);
INT32 PPPP_DRWAck_Read_Header(st_PPPP_DRWAckHeader* pDRWAckHeader, UCHAR *Channel, UINT16 *NumberOfIndex);
void PPPP_Proto_Write_SDevLgn(st_PPPP_SDevLgn *pSDevLgn, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Read_SDevLgn(st_PPPP_SDevLgn *pSDevLgn, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Write_ListReq1(st_PPPP_ListReq1 *pListReq1, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Read_ListReq1(st_PPPP_ListReq1 *pListReq1, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Write_MGMDumpLoginDID1(st_PPPP_MGMDumpLoginDID1 *pMGMDumpLoginDID1, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Read_MGMDumpLoginDID1(st_PPPP_MGMDumpLoginDID1 *pMGMDumpLoginDID1, CHAR *Prefix, UINT32 *SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Write_MGMRemoteManagement(st_PPPP_MGMRemoteManagement *pMGMRemoteManagementHeader, UCHAR FunctionID, UCHAR CMDorRESP, UCHAR PacketIndex, UCHAR PacketTotalNum, UINT16 DataSize);
INT32 PPPP_Proto_Read_MGMRemoteManagement(st_PPPP_MGMRemoteManagement* pMGMRemoteManagementHeader, UCHAR *FunctionID, UCHAR *CMDorRESP, UCHAR *PacketIndex, UCHAR *PacketTotalNum, UINT16 *DataSize);

INT32 PPPP_Proto_Recv_ALL(INT32 skt, struct sockaddr_in *FromAddr, UINT32 TimeOut_ms, UCHAR *MsgType, UINT16 *Size, CHAR *Data, UINT16 MaxDataSize);
INT32 PPPP_Proto_Send_Hello(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_HelloAck(INT32 skt, struct sockaddr_in *ToAddr, struct sockaddr_in *RemoteAddr);
INT32 PPPP_Proto_Send_DevLgn(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, CHAR NATType, UCHAR *APIVersion,	struct sockaddr_in *LocalAddr);
INT32 PPPP_Proto_Send_DevLgnAck(INT32 skt, struct sockaddr_in *ToAddr, CHAR Result);
INT32 PPPP_Proto_Send_DevLgn_CRC(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, CHAR NATType, UCHAR *APIVersion,	struct sockaddr_in *LocalAddr, CHAR *CRCKey);  //// ver 1.2
INT32 PPPP_Proto_Send_DevLgnAck_CRC(INT32 skt, struct sockaddr_in *ToAddr, CHAR Result, CHAR *CRCKey);//// ver 1.2
INT32 PPPP_Proto_Send_P2PReq(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, struct sockaddr_in *LocalAddr);
INT32 PPPP_Proto_Send_P2PReqAck(INT32 skt, struct sockaddr_in *ToAddr, CHAR Result);
INT32 PPPP_Proto_Send_PunchTo(INT32 skt, struct sockaddr_in *ToAddr, struct sockaddr_in *RemoteAddr);
INT32 PPPP_Proto_Send_RSLgn(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, UINT32 BandWidth, UINT32 UserNumber);
INT32 PPPP_Proto_Send_RSLgnAck(INT32 skt, struct sockaddr_in *ToAddr, CHAR Result);
INT32 PPPP_Proto_Send_RlyHello(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_RlyHelloAck(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_RlyReq(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, struct sockaddr_in *RLYAddr, UINT32 MagicWord);
INT32 PPPP_Proto_Send_RlyReqAck(INT32 skt, struct sockaddr_in *ToAddr, CHAR Result);
INT32 PPPP_Proto_Send_RlyTo(INT32 skt, struct sockaddr_in *ToAddr, struct sockaddr_in *RlyToAddr, UINT32 MagicWord);
INT32 PPPP_Proto_Send_ListReq(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_ListReqAck(INT32 skt, struct sockaddr_in *ToAddr, UCHAR ListNumber, struct sockaddr_in *AddrList);
INT32 PPPP_Proto_Send_RlyPort(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_RlyPortAck(INT32 skt, struct sockaddr_in *ToAddr, UINT32 MagicWord, UINT16 Port);
INT32 PPPP_Proto_Send_ByteCount(INT32 skt, struct sockaddr_in *ToAddr, UINT32 Count);
INT32 PPPP_Proto_Send_RlyPkt(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, CHAR CorD,UINT32 MagicWord);
INT32 PPPP_Proto_Send_PunchPkt(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
INT32 PPPP_Proto_Send_P2PRdy(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
void PPPP_Proto_Send_LanSerch(INT32 skt, UINT16 Port);
INT32 PPPP_Proto_Send_RlyRdy(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
INT32 PPPP_DRW_Send(INT32 skt, struct sockaddr_in *ToAddr, UCHAR Channel, UINT16 Index,CHAR *DataBuf, UINT16 DataSize);
INT32 PPPP_Proto_Send_Alive(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_AliveAck(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_Close(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_MGMDumpLoginDID(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_MGMDumpLoginDIDDetail(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_DRWAck_Send(INT32 skt, struct sockaddr_in *ToAddr, UCHAR Channel, UINT16 *pIndex, UINT16 NumberOfIndex);
INT32 PPPP_Proto_Send_SDevRun(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_HelloToAck(INT32 skt, struct sockaddr_in *ToAddr);
INT32 PPPP_Proto_Send_HelloTo(INT32 skt, struct sockaddr_in *ToAddr, struct sockaddr_in *TargetAddr1, struct sockaddr_in *TargetAddr2);
INT32 PPPP_Proto_Send_SDevLgn(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
INT32 PPPP_Proto_Send_SDevLgnAck(INT32 skt, struct sockaddr_in *ToAddr, struct sockaddr_in *WanAddr);
INT32 PPPP_Proto_Send_SDevLgn_CRC(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode, CHAR *CRCKey); //// ver 1.2
INT32 PPPP_Proto_Send_SDevLgnAck_CRC(INT32 skt, struct sockaddr_in *ToAddr, struct sockaddr_in *WanAddr, CHAR *CRCKey); //// ver 1.2
INT32 PPPP_Proto_Send_ListReq1(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
INT32 PPPP_Proto_Send_QueryDID(INT32 skt, struct sockaddr_in *ToAddr, const CHAR* DeviceName);
INT32 PPPP_Proto_Send_QueryDIDAck(INT32 skt, struct sockaddr_in *ToAddr, const CHAR* DID);
INT32 PPPP_Proto_Send_MGMDumpLoginDID1(INT32 skt, struct sockaddr_in *ToAddr, CHAR *Prefix, UINT32 SerialNumber, CHAR *CheckCode);
INT32 PPPP_Proto_Send_MGMLogControl(INT32 skt, struct sockaddr_in *ToAddr, CHAR Command);
INT32 PPPP_Proto_Send_MGMRemoteManagement(INT32 skt, struct sockaddr_in *ToAddr, UCHAR FunctionID, UCHAR CMDorRESP, CHAR *DataBuf, UINT16 DataSize);
#endif  //#ifndef __INCLUDE_FILE_PPPP_PROTOCOL


