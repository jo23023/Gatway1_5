#ifndef SERIALPORT_H_
#define SERIALPORT_H_

union RF_DATA_FORMAT{
  struct RF_DATA{
   
	 UINT32 Addressid;
	 UCHAR nodeid;
	 UCHAR  devicestype;
	 UCHAR misc;
	 UCHAR   controlbit;
	 UCHAR   data2;
	 UCHAR   data1;
	 UCHAR   data0;
	 UCHAR checksum;
	 
  }	FORMAT;
    UCHAR data[12];
};


int open_ports(int comport);
int set_opt(int nSpeed, int nBits, char nEvent, int nStop);

#endif
