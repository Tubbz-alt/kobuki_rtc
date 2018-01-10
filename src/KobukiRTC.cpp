// -*- C++ -*-
/*!
 * @file  KobukiRTC.cpp
 * @brief Kobuki RTC
 * @date $Date$
 *
 * @license MIT lisence
 */

#include "KobukiRTC.h"

// Module specification
// <rtc-template block="module_spec">
static const char* kobukirtc_spec[] =
  {
    "implementation_id", "KobukiRTC",
    "type_name",         "KobukiRTC",
    "description",       "Kobuki RTC",
    "version",           "1.0.0",
    "vendor",            "Mizukawa Lab. Shibaura Univ.",
    "category",          "Experimenta",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.debug", "0",
#ifdef WIN32
    "conf.default.port", "COM1",
#else
    "conf.default.port", "/dev/ttyUSB0",
#endif
    // Widget
    "conf.__widget__.debug", "text",
    // Constraints
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
KobukiRTC::KobukiRTC(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_targetVelocityIn("targetVelocity", m_targetVelocity),
    m_poseUpdateIn("poseUpdate", m_poseUpdate),
    m_currentPoseOut("currentPose", m_currentPose),
    m_batteryOut("battery", m_battery),
    m_bumperOut("bumper", m_bumper),
	m_commandIn("command", m_command),
	m_dockStateOut("dockState", m_dockState)

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
KobukiRTC::~KobukiRTC()
{
}



RTC::ReturnCode_t KobukiRTC::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("targetVelocity", m_targetVelocityIn);
  addInPort("poseUpdate", m_poseUpdateIn);
  addInPort("command", m_commandIn);
  
  // Set OutPort buffer
  addOutPort("currentPose", m_currentPoseOut);
  addOutPort("battery", m_batteryOut);
  addOutPort("bumper", m_bumperOut);
  addOutPort("dockState", m_dockStateOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("debug", m_debug, "0");
  bindParameter("port", m_port, "COM1");
  // </rtc-template>
  
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t KobukiRTC::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t KobukiRTC::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t KobukiRTC::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

std::string getDockStateStr(const DOCKSTATE s) {
	switch (s) {
	case IDLE: return "IDLE";
	case DOCKED: return "DOCKED";
	case DOCKING: return "DOCKING";
	case BUMPSTOP: return "BUMPSTOP";
	case CLIFFSTOP: return "CLIFFSTOP";
	case DROPSTOP: return "DROPSTOP";
	case LOSTSTOP: return "LOSTSTOP";
	default: return "UNKNOWN";
	}
}
RTC::ReturnCode_t KobukiRTC::onActivated(RTC::UniqueId ec_id)
{
  m_pKobuki = createKobuki(rt_net::KobukiStringArgument(m_port));
  m_bumper.data.length(3);

  m_dockStateBuf = m_pKobuki->getDockState();
  m_dockState.data = CORBA::string_dup(getDockStateStr(m_dockStateBuf).c_str());
  setTimestamp(m_dockState);
  m_dockStateOut.write();
  m_bumper.data.length(3);
  return RTC::RTC_OK;
}


RTC::ReturnCode_t KobukiRTC::onDeactivated(RTC::UniqueId ec_id)
{
  delete m_pKobuki;
  return RTC::RTC_OK;
}


RTC::ReturnCode_t KobukiRTC::onExecute(RTC::UniqueId ec_id)
{
  if(m_targetVelocityIn.isNew()) {
    m_targetVelocityIn.read();
    m_pKobuki->setTargetVelocity(m_targetVelocity.data.vx, m_targetVelocity.data.va);
  }

  if(m_poseUpdateIn.isNew()) {
    m_poseUpdateIn.read();
    m_pKobuki->setPose(m_poseUpdate.data.position.x, m_poseUpdate.data.position.y, m_poseUpdate.data.heading);
  }

  m_currentPose.data.position.x = m_pKobuki->getPoseX();
  m_currentPose.data.position.y = m_pKobuki->getPoseY();
  m_currentPose.data.heading = m_pKobuki->getPoseTh();
  m_currentPoseOut.write();
  
  m_battery.data = m_pKobuki->getBatteryVoltage();
  m_batteryOut.write();
  
  m_bumper.data[0] = m_pKobuki->isRightBump();
  m_bumper.data[1] = m_pKobuki->isCenterBump();
  m_bumper.data[2] = m_pKobuki->isLeftBump();
  m_bumperOut.write();
  
  DOCKSTATE ds = m_pKobuki->getDockState();
  if (ds != m_dockStateBuf) {
	  m_dockState.data = CORBA::string_dup(getDockStateStr(ds).c_str());
	  setTimestamp(m_dockState);
	  m_dockStateOut.write();
	  m_dockStateBuf = ds;
  }

  if (m_commandIn.isNew()) {
	  m_commandIn.read();
	  std::string command = m_command.data;
	  if (command == "DOCK") {
		  m_pKobuki->dock(false);
	  }
  }
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t KobukiRTC::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t KobukiRTC::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t KobukiRTC::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t KobukiRTC::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t KobukiRTC::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void KobukiRTCInit(RTC::Manager* manager)
  {
    coil::Properties profile(kobukirtc_spec);
    manager->registerFactory(profile,
                             RTC::Create<KobukiRTC>,
                             RTC::Delete<KobukiRTC>);
  }
  
};


