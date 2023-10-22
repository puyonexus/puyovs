// Copyright (c) 2023, LossRain
// SPDX-License-Identifier: GPLv3

#pragma once

#include <string>
#include <deque>
#include <iostream>
#include <functional>

namespace ppvs {

enum class DebugMessageType {
	NONE = 0,
	ERROR = 1,
	WARNING = 2,
	INFO = 4,
	DEBUG = 8
};

/* A more precise debug message class, might be used in exception handling and other scenarios
 * where we don't want to handle raw strings.
 * */
class DebugMessage {
public:
	DebugMessage(std::basic_string<char> text, DebugMessageType severity = DebugMessageType::NONE) : m_text(text), m_severity(severity) {};
	~DebugMessage() = default;

	std::string m_text;
	DebugMessageType m_severity;
};


/* Puyolib debugging. This class allows a developer to send debug messages from Puyolib to the Client.
 *
 * A developer seeking to send a debug message to the client shall use the log method (under normal circumstances
 * a member of ppvs::Game named m_debug) and call the log method with a std::string and
 * an appropriate Debug Message type.
 * */
class DebugLog {
public:
	DebugLog() = default;
	~DebugLog();

	void log(DebugMessage *msg);
	void log(std::string text, DebugMessageType severity);

	void setLogHandler(std::function<void(std::basic_string<char>,DebugMessageType)> f);

private:
	std::function<void(std::string,DebugMessageType)> m_log_handler;


};

} // ppvs

