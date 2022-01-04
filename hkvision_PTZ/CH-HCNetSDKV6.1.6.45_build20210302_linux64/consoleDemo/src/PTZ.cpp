/*
* Copyright(C) 2010,Hikvision Digital Technology Co., Ltd
*
* File   name��CapPicture.cpp
* Discription��
* Version    ��1.0
* Author     ��panyd
* Create Date��2010_3_25
* Modification History��
*/
#include <stdio.h>
#include <iostream>
#include "GetStream.h"
#include "public.h"
#include "ConfigParams.h"
#include "Alarm.h"
#include "CapPicture.h"
#include "Control.h"
#include "controlrotation.h"
#include "tool.h"
#include <string.h>
#include <unistd.h>
#include <vector>
using namespace std;

//相机的角度参数是十六进制,需要和十进制来回切换
int  HexToDecMa(int wHex)//十六进制转十进制
{
    return (wHex / 4096) * 1000 + ((wHex % 4096) / 256) * 100 + ((wHex % 256) / 16) * 10 + (wHex % 16);
}

int DEC2HEX_doc(int x)//十进制转十六进制
{
    return (x / 1000) * 4096 + ((x % 1000) / 100) * 256 + ((x % 100) / 10) * 16 + x % 10;
}
/**
 * @brief ControlCamera
 * @return   主要控制函数
 */
int ControlCamera()
{
    NET_DVR_Init();
    long lUserID;
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    //login注册相机
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
    struLoginInfo.bUseAsynLogin = false;

    struLoginInfo.wPort = 8000;
    memcpy(struLoginInfo.sDeviceAddress, "192.168.1.64", NET_DVR_DEV_ADDRESS_MAX_LEN);
    memcpy(struLoginInfo.sUserName, "admin", NAME_LEN);
    memcpy(struLoginInfo.sPassword, "password", NAME_LEN);
      //login
    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
      //读取相机信息
          NET_DVR_PTZPOS m_ptzPosCurrent;
           DWORD dwtmp;
           bool a = NET_DVR_GetDVRConfig(0, NET_DVR_GET_PTZPOS, 0, &m_ptzPosCurrent, sizeof(NET_DVR_PTZPOS), &dwtmp);
      //转换相机信息到十进制
           int m_iPara1 = HexToDecMa(m_ptzPosCurrent.wPanPos);
           int m_iPara2 = HexToDecMa(m_ptzPosCurrent.wTiltPos);
           int m_iPara3 = HexToDecMa(m_ptzPosCurrent.wZoomPos);
           vector<int> TempPosture(3);//三个参数代表含义如下cout所示
           TempPosture[0] = m_iPara1 / 10 + 1 ;
           TempPosture[1] = m_iPara2 / 10 + 1;
           TempPosture[2] = m_iPara3 / 10 ;

    //输出此时相机的三个参数
           cout << "##################beforecontrol#####################"<< endl;
           cout << "P水平方向   " << TempPosture[0]  <<   "                   #" << endl;
           cout << "T仰角      " << TempPosture[1]  <<   "                   #" << endl;
           cout << "Z焦距      " << TempPosture[2]  <<      "                   #" << endl;
           cout << "###############################################"<< endl;
         while(1){
         //设置想要变化的参数值
           vector<int> Changevalue(3);
           Changevalue[0]=(30);//degree水平加30度
           Changevalue[1]=(-10);//degree俯仰减10度
           Changevalue[2]= (1);//焦距加1
//定义目标参数
           vector<int> TargetPosture(3);
//将变化值更新
           TargetPosture[0]=TempPosture[0]+Changevalue[0];
           TargetPosture[1]=TempPosture[1]+Changevalue[1];
           TargetPosture[2]=TempPosture[2]+Changevalue[2];

 //          m_ptzPosCurrent.wPanPos = DEC2HEX_doc(1800);
//           m_ptzPosCurrent.wTiltPos = DEC2HEX_doc(0);
//           m_ptzPosCurrent.wZoomPos = DEC2HEX_doc(1);
      //将变化后的参数转化为相机自带的十六进制
           m_ptzPosCurrent.wPanPos = DEC2HEX_doc(TargetPosture[0]*10-1);
           m_ptzPosCurrent.wTiltPos = DEC2HEX_doc(TargetPosture[1]*10-1);
           m_ptzPosCurrent.wZoomPos = DEC2HEX_doc(TargetPosture[2]*10);

          //更新此时的位置参数
           TempPosture[0] = TargetPosture[0];
           TempPosture[1] = TargetPosture[1];
           TempPosture[2] = TargetPosture[2];
    //将参数传入函数,相机开始调整位置
         bool b=NET_DVR_SetDVRConfig(0, NET_DVR_SET_PTZPOS, 0, &m_ptzPosCurrent, sizeof(NET_DVR_PTZPOS));
           sleep(2);
           
      //相机控制成功,接下来的代码根据需求看要不要
           //second back反过来回到原位
           Changevalue[0]=(-30);//degree
           Changevalue[1]=(10);//degree
           Changevalue[2]= (0);//焦距

           TargetPosture[0]=TempPosture[0]+Changevalue[0];
           TargetPosture[1]=TempPosture[1]+Changevalue[1];
           TargetPosture[2]=TempPosture[2]+Changevalue[2];
           m_ptzPosCurrent.wPanPos = DEC2HEX_doc(TargetPosture[0]*10);
           m_ptzPosCurrent.wTiltPos = DEC2HEX_doc(TargetPosture[1]*10);
           m_ptzPosCurrent.wZoomPos = DEC2HEX_doc(TargetPosture[2]*10);

         NET_DVR_SetDVRConfig(0, NET_DVR_SET_PTZPOS, 0, &m_ptzPosCurrent, sizeof(NET_DVR_PTZPOS));

         if (!b){
                     auto a = NET_DVR_GetLastError();
         }

}

         cout << "##################aftercontrol#####################"<< endl;
         cout << "P水平方向   " << TempPosture[0]  <<   "                   #" << endl;
         cout << "T仰角      " << TempPosture[1]  <<   "                   #" << endl;
         cout << "Z焦距      " << TempPosture[2]  <<      "                   #" << endl;
         cout << "###############################################"<< endl;
        /*
         //control by rotate time
          // successcontroled =
    //        NET_DVR_PTZControl(lRealPlayHandle,dwPTZCommand, dwStop);
  // sleep(1);

   // if (!successcontroled)
  //  {
  //      printf("faild to control the PTZ, %d\n", NET_DVR_GetLastError());

 //   }
   //  NET_DVR_PTZControl(lRealPlayHandle,dwPTZCommand, !dwStop);

   //关闭预览
 //    NET_DVR_StopRealPlay(lRealPlayHandle);
    //logout
    */
    NET_DVR_Logout_V30(lUserID);
    NET_DVR_Cleanup();

    return HPR_OK;

}

