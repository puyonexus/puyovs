// Copyright (c) 2023, LossRain
// SPDX-License-Identifier: GPLv3

#include "DebugLog.h"

namespace ppvs {

DebugLog::~DebugLog() {
	m_log_handler = NULL;
}

void DebugLog::setLogHandler(std::function<void(std::basic_string<char>, DebugMessageType)> func)
{
	m_log_handler = func;
	log("Debug Log was successfully linked", DebugMessageType::DEBUG);
}

void DebugLog::log(ppvs::DebugMessage* msg)
{
	if (m_log_handler != NULL) {
		m_log_handler(msg->m_text, msg->m_severity);
	}
	delete msg;
}

void DebugLog::log(std::string text, DebugMessageType severity)
{
	if (m_log_handler != NULL) {
		m_log_handler(text, severity);
	}
}

} // ppvs