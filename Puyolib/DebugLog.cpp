// Copyright (c) 2023, LossRain
// SPDX-License-Identifier: GPLv3

#include "DebugLog.h"

namespace ppvs {

DebugLog::~DebugLog() {
	// Assumes that log handler isn't dead yet
	log("Debug Log was successfully destroyed", DebugMessageType::INFO);
	m_log_handler = NULL;
}

void DebugLog::setLogHandler(std::function<void(std::basic_string<char>, DebugMessageType)> func)
{
	m_log_handler = func;
	log("Debug Log was successfully created", DebugMessageType::INFO);
}

/* Sends the Debug message to the client.
 * Before calling: make sure that the logging handler is deployed and bound.
 * */
void DebugLog::log(ppvs::DebugMessage* msg)
{
	if (m_log_handler != NULL) {
		m_log_handler(msg->m_text, msg->m_severity);
	}
	delete msg;
}

/* Sends the text to the client to display as debug output. The text should be a std::string object.
 * Before calling: make sure that the logging handler is deployed and bound.
 * */
void DebugLog::log(std::string text, DebugMessageType severity = DebugMessageType::DEBUG)
{
	if (m_log_handler != NULL) {
		m_log_handler(text, severity);
	}
}

} // ppvs