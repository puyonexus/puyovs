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

class DebugMessage {
public:
	DebugMessage(std::basic_string<char> text, DebugMessageType severity = DebugMessageType::NONE) : m_text(text), m_severity(severity) {};
	~DebugMessage() = default;

	std::string m_text;
	DebugMessageType m_severity;
};

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

