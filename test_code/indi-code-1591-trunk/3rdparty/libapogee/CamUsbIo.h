/*! 
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright(c) 2009 Apogee Instruments, Inc. 
* \class CamUsbIo 
* \brief Usb implemenation of the ICamIo interface 
* 
*/ 


#ifndef CAMUSBIO_INCLUDE_H__ 
#define CAMUSBIO_INCLUDE_H__ 

#include "ICamIo.h"
#include "CameraStatusRegs.h"

#include "CamHelpers.h" 

#include <string>
#include <vector>
#ifdef WIN_OS
#include <memory>
#else
#include <tr1/memory>
#endif

class IUsb;


class CamUsbIo : public ICamIo
{ 
    public: 
        virtual ~CamUsbIo(); 

        uint16_t ReadReg( uint16_t reg ) const;
	    void WriteReg( uint16_t reg, uint16_t val ) ;

        void WriteSRMD( uint16_t reg, const std::vector<uint16_t> & data );
        void WriteMRMD( uint16_t reg, const std::vector<uint16_t> & data );

        void GetUsbVendorInfo( uint16_t & VendorId,
            uint16_t & ProductId, uint16_t  & DeviceId);

        std::string GetUsbFirmwareVersion();

        void SetupImgXfer(uint16_t Rows, 
            uint16_t Cols, 
            uint16_t NumOfImages, 
            bool IsBulkSeq);

        void CancelImgXfer();
       
        void GetImageData( std::vector<uint16_t> & data );
    
        void GetStatus(CameraStatusRegs::BasicStatus & status);
        void GetStatus(CameraStatusRegs::AdvStatus & status);

        uint16_t GetFirmwareRev();

         std::string GetInfo();

        uint8_t ReadBufConReg( uint16_t reg );
	    void WriteBufConReg( uint16_t reg, uint8_t val );

        uint8_t ReadFx2Reg( uint16_t reg );
        void WriteFx2Reg( uint16_t reg, uint8_t val );

        uint32_t GetMaxXferBufSize() { return m_MaxBufSize; }

        std::string GetDriverVersion();

         bool IsError();

        ////////////////////////////////////////////////////////////////////
        // PURE VIRTUALS
        virtual void SetSerialNumber(const std::string & num) = 0;
        virtual std::string GetSerialNumber() = 0;
        virtual void ReadHeader( Eeprom::Header & hdr ) = 0;

    protected:
        CamUsbIo( const std::string & DeviceEnum, 
            uint32_t MaxBufSize,
            bool ApplyPad );

        void Progress2StdOut(const int32_t percentComplete);

        ////////////////////////////////////////////////////////////////////
        // MEMBER VARIABLES

        bool m_Print2StdOut;

         std::tr1::shared_ptr<IUsb> m_Usb; 
        const std::string m_fileName;
        bool m_ApplyPadding;
        uint32_t m_MaxBufSize;

        int32_t GetPadding( const int32_t Num );

        private:
        //disabling the copy ctor and assignment operator
        //generated by the compiler - don't want them
        //Effective C++ Item 6
        CamUsbIo(const CamUsbIo&);
        CamUsbIo& operator=(CamUsbIo&);
}; 

#endif
