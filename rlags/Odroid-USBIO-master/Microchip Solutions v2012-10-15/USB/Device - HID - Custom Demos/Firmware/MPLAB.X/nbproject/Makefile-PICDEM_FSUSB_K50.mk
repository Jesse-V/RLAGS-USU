#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-PICDEM_FSUSB_K50.mk)" "nbproject/Makefile-local-PICDEM_FSUSB_K50.mk"
include nbproject/Makefile-local-PICDEM_FSUSB_K50.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=PICDEM_FSUSB_K50
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=cof
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/MPLAB.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/MPLAB.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/926206843/usb_device.o ${OBJECTDIR}/_ext/1785203669/usb_function_hid.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/926206843/usb_device.o.d ${OBJECTDIR}/_ext/1785203669/usb_function_hid.o.d ${OBJECTDIR}/_ext/1472/main.o.d ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/926206843/usb_device.o ${OBJECTDIR}/_ext/1785203669/usb_function_hid.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE} ${MAKE_OPTIONS} -f nbproject/Makefile-PICDEM_FSUSB_K50.mk dist/${CND_CONF}/${IMAGE_TYPE}/MPLAB.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F45K50
MP_PROCESSOR_OPTION_LD=18f45k50
MP_LINKER_DEBUG_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/926206843/usb_device.o: ../../../../Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/926206843 
	@${RM} ${OBJECTDIR}/_ext/926206843/usb_device.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG  -p$(MP_PROCESSOR_OPTION) -DHARDKERNEL_PIC18F45K50 -I"C:/Microchip Solutions v2012-10-15/Microchip/Include" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware/USB" -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/926206843/usb_device.o   ../../../../Microchip/USB/usb_device.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/926206843/usb_device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/926206843/usb_device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1785203669/usb_function_hid.o: ../../../../Microchip/USB/HID\ Device\ Driver/usb_function_hid.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1785203669 
	@${RM} ${OBJECTDIR}/_ext/1785203669/usb_function_hid.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG  -p$(MP_PROCESSOR_OPTION) -DHARDKERNEL_PIC18F45K50 -I"C:/Microchip Solutions v2012-10-15/Microchip/Include" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware/USB" -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1785203669/usb_function_hid.o   "../../../../Microchip/USB/HID Device Driver/usb_function_hid.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1785203669/usb_function_hid.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1785203669/usb_function_hid.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG  -p$(MP_PROCESSOR_OPTION) -DHARDKERNEL_PIC18F45K50 -I"C:/Microchip Solutions v2012-10-15/Microchip/Include" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware/USB" -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/main.o   ../main.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG  -p$(MP_PROCESSOR_OPTION) -DHARDKERNEL_PIC18F45K50 -I"C:/Microchip Solutions v2012-10-15/Microchip/Include" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware/USB" -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/usb_descriptors.o   ../usb_descriptors.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
else
${OBJECTDIR}/_ext/926206843/usb_device.o: ../../../../Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/926206843 
	@${RM} ${OBJECTDIR}/_ext/926206843/usb_device.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DHARDKERNEL_PIC18F45K50 -I"C:/Microchip Solutions v2012-10-15/Microchip/Include" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware/USB" -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/926206843/usb_device.o   ../../../../Microchip/USB/usb_device.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/926206843/usb_device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/926206843/usb_device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1785203669/usb_function_hid.o: ../../../../Microchip/USB/HID\ Device\ Driver/usb_function_hid.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1785203669 
	@${RM} ${OBJECTDIR}/_ext/1785203669/usb_function_hid.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DHARDKERNEL_PIC18F45K50 -I"C:/Microchip Solutions v2012-10-15/Microchip/Include" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware/USB" -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1785203669/usb_function_hid.o   "../../../../Microchip/USB/HID Device Driver/usb_function_hid.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1785203669/usb_function_hid.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1785203669/usb_function_hid.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DHARDKERNEL_PIC18F45K50 -I"C:/Microchip Solutions v2012-10-15/Microchip/Include" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware/USB" -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/main.o   ../main.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -DHARDKERNEL_PIC18F45K50 -I"C:/Microchip Solutions v2012-10-15/Microchip/Include" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware" -I"C:/Microchip_Solutions_v2012-10-15/USB/dev_hid_demo/Firmware/USB" -ms -oa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/usb_descriptors.o   ../usb_descriptors.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c18 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/MPLAB.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  ../p18f45k50.lib  ../rm18f45k50_g.lkr
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) "..\rm18f45k50_g.lkr"  -p$(MP_PROCESSOR_OPTION_LD)  -w -x -u_DEBUG -m"$(BINDIR_)$(TARGETBASE).map"  -z__MPLAB_BUILD=1  -u_CRUNTIME -z__MPLAB_DEBUG=1 $(MP_LINKER_DEBUG_OPTION) -l ${MP_CC_DIR}\\..\\lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/MPLAB.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}  ..\p18f45k50.lib 
else
dist/${CND_CONF}/${IMAGE_TYPE}/MPLAB.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk  ../p18f45k50.lib ../rm18f45k50_g.lkr
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) "..\rm18f45k50_g.lkr"  -p$(MP_PROCESSOR_OPTION_LD)  -w  -m"$(BINDIR_)$(TARGETBASE).map"  -z__MPLAB_BUILD=1  -u_CRUNTIME -l ${MP_CC_DIR}\\..\\lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/MPLAB.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}  ..\p18f45k50.lib 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/PICDEM_FSUSB_K50
	${RM} -r dist/PICDEM_FSUSB_K50

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
