/**
 * \file GuiChatMessenger.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Tommaso Cucinotta
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <qxmpp/QXmppClient.h>

#include "GuiChatMessenger.h"
#include "GuiChat.h"
#include "GuiBuddies.h"

#include "qxmpp/QXmppMessage.h"
#include "qxmpp/QXmppRosterManager.h"

#include <support/debug.h>
#include "support/lassert.h"
#include "support/FileName.h"
#include "support/Package.h"
#include "support/filetools.h"

namespace lyx {
namespace frontend {

using namespace std;
using namespace support;

FileName GuiChatMessenger::chatsDir()
{
	FileName chats_dir(addPath(package().user_support().absFileName(), "chats"));
	if (!chats_dir.exists() && !chats_dir.createDirectory(0777)) {
		LYXERR(Debug::GUI, "LyX could not create the user chats directory '"
		       << chats_dir << "'. Chats history will not be kept.");
	}
	if (!chats_dir.isDirWritable()) {
		LYXERR(Debug::GUI, "LyX could not write to the user chats directory '"
		       << chats_dir << "'. Chats history will not be kept.");
	}
	return chats_dir;
}


FileName GuiChatMessenger::chatFileName(const std::string & peername)
{
	return FileName(chats_dir, peername + ".lyx");
}


FileName GuiChatMessenger::passwordFileName(const std::string & username)
{
	return FileName(chats_dir, username + ".passwd");
}


GuiChatMessenger::GuiChatMessenger()
	: p_chat(0), p_buddies(0), qxmpp_client(*new QXmppClient())
{
	chats_dir = chatsDir();

	connect(&qxmpp_client, SIGNAL(messageReceived(const QXmppMessage&)),
	 	SLOT(messageReceived(const QXmppMessage&)));
}


string GuiChatMessenger::cutResource(string const & id)
{
	size_t pos = id.find('/');
	if (pos != string::npos)
		return id.substr(0, pos);
	return id;
}


void GuiChatMessenger::messageReceived(const QXmppMessage& message)
{
	QString from = message.from();
	QString msg = message.body();

	string from_s = cutResource(fromqstr(from));
	lyxerr << "Received from " << from << " (" << from_s << "): " << msg << endl;
	if (p_chat)
		p_chat->onMessageReceived(from_s, fromqstr(msg));
}


void GuiChatMessenger::sendMessage(
	const std::string & from, const std::string & dest,
	const std::string & msg)
{
	qxmpp_client.sendPacket(QXmppMessage(toqstr(from), toqstr(dest), toqstr(msg)));
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiChatMessenger.cpp"
