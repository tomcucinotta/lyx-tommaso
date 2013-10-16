// -*- C++ -*-
/**
 * \file Chat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Tommaso Cucinotta
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QGUICHATMESSENGER_H
#define QGUICHATMESSENGER_H

#include <config.h>

#include <qobject.h>
#include "support/FileName.h"

#include <string>

class QXmppClient;
class QXmppMessage;

namespace lyx {
namespace frontend {

class GuiChat;
class GuiChatWidget;
class GuiBuddiesWidget;
class GuiView;

class GuiChatMessenger : QObject
{
	Q_OBJECT

public:
	GuiChatMessenger();
	void sendMessage(const std::string & from, const std::string & dest, const std::string & msg);

	support::FileName chatsDir();

	support::FileName chatFileName(const std::string & peername);
	support::FileName passwordFileName(const std::string & username);

	support::FileName chats_dir;
	GuiChatWidget *p_chat;
	GuiBuddiesWidget *p_buddies;

	static std::string cutResource(std::string const & id);

	///
	QXmppClient & qxmpp_client;

protected Q_SLOTS:
	void messageReceived(const QXmppMessage&);
};

} // namespace frontend
} // namespace lyx

#endif // QGUICHATMESSENGER_H
