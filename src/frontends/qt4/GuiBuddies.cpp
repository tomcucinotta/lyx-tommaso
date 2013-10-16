/**
 * \file GuiBuddies.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Tommaso Cucinotta
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiBuddies.h"
#include "GuiChat.h"
#include "LyXRC.h"

#include "GuiApplication.h"
#include "GuiView.h"
#include "GuiWorkArea.h"
#include "qt_helpers.h"
#include "Language.h"
#include <QWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>

#include "FuncRequest.h"
#include "lyxfind.h"

#include "frontends/alert.h"
#include "DispatchResult.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/Package.h"

#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppRosterManager.h>

#include <fstream>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiBuddiesWidget::GuiBuddiesWidget(GuiView & view)
	: QTabWidget(&view), view_(view)
{
	setupUi(this);

	connect(buddiesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
		SLOT(onItemDoubleClicked(QListWidgetItem*)));

	connect(buddiesList, SIGNAL(itemClicked(QListWidgetItem*)),
		SLOT(onItemClicked(QListWidgetItem*)));

	connect(&view_.chatMessenger().qxmpp_client,
		SIGNAL(stateChanged(QXmppClient::State)),
		this, SLOT(onStateChanged(QXmppClient::State)));

	connect(&view_.chatMessenger().qxmpp_client.rosterManager(),
		SIGNAL(presenceChanged(const QString&, const QString&)),
		this, SLOT(onPresenceChanged(const QString&, const QString&)));

	connect(&view_.chatMessenger().qxmpp_client.rosterManager(),
		SIGNAL(itemRemoved(const QString&)),
		this, SLOT(onItemRemoved(const QString&)));

	connect(&view_.chatMessenger().qxmpp_client.rosterManager(),
		SIGNAL(itemAdded(const QString&)),
		this, SLOT(onItemAdded(const QString&)));

	connect(&view_.chatMessenger().qxmpp_client.rosterManager(),
		SIGNAL(subscriptionReceived(const QString&)),
		this, SLOT(onSubscriptionReceived(const QString&)));

	connect(userEdit, SIGNAL(returnPressed()),
	 	this, SLOT(on_connectPB_clicked()));

	connect(statusCO, SIGNAL(activated(const QString&)),
	 	this, SLOT(onStateSelected(const QString &)));

	username = toqstr(lyxrc.user_chat_id);
	if (username.size() > 0) {
		userEdit->setText(username);
		password = toqstr(getPasswordFor(fromqstr(username)));
		if (password.size() > 0) {
			view_.chatMessenger().qxmpp_client.connectToServer(username, password);
			statusLabel->setText(qt_("Connecting..."));
		}
	}

	icon_avail = QIcon::fromTheme("available", getPixmap("images/", "available", "png"));
	icon_offline = QIcon::fromTheme("offline", getPixmap("images/", "offline", "png"));
	icon_away = QIcon::fromTheme("away", getPixmap("images/", "away", "png"));
	icon_dnd = QIcon::fromTheme("donotdisturb", getPixmap("images/", "donotdisturb", "png"));
}


GuiBuddiesWidget::~GuiBuddiesWidget()
{
	view_.chatMessenger().qxmpp_client.disconnectFromServer();
}


string GuiBuddiesWidget::getPasswordFor(const string & username)
{
	FileName fname = view_.chatMessenger().passwordFileName(username);
	if (!fname.exists())
		return string();
	ifstream ifs(fname.absFileName().c_str());
	char pwd[80];
	ifs.getline(pwd, sizeof(pwd));
	ifs.close();
	return string(pwd);
}


void GuiBuddiesWidget::storePasswordFor(const string & username, const string & pwd)
{
	FileName fname = view_.chatMessenger().passwordFileName(username);
	ofstream ofs(fname.absFileName().c_str());
	ofs << pwd << std::endl;
	ofs.close();
}


bool GuiBuddiesWidget::initialiseParams(std::string const & /*params*/)
{
	return true;
}


static QListWidgetItem *findInListWidget(QListWidget *p_list, const QString &s) {
	for (int i = 0; i < p_list->count(); ++i)
		if (p_list->item(i)->text() == s)
			return p_list->item(i);
	return 0;
}


QIcon GuiBuddiesWidget::presenceToIcon(const QXmppPresence & pres) {
	if (pres.type() == QXmppPresence::Unavailable)
		return icon_offline;
	if (pres.availableStatusType() == QXmppPresence::Online
	    || pres.availableStatusType() == QXmppPresence::Chat)
		return icon_avail;
	if (pres.availableStatusType() == QXmppPresence::DND)
		return icon_dnd;
	return icon_away;
}


void GuiBuddiesWidget::onPresenceChanged(const QString& id, const QString &res)
{
	QXmppPresence pres = view_.chatMessenger().qxmpp_client.rosterManager().getPresence(id, res);
	// Ignore subscribe/unsubscribe/probe messages, requesting an action/reaction
	if (pres.type() != QXmppPresence::Available
	    && pres.type() != QXmppPresence::Unavailable)
		return;

	if (id == username) {
		// TBD
		return;
	}

	QListWidgetItem *p_item = findInListWidget(buddiesList, id);
	if (p_item) {
		p_item->setIcon(presenceToIcon(pres));
	} else {
		p_item = new QListWidgetItem(presenceToIcon(pres), id, buddiesList);
	}
}


void GuiBuddiesWidget::onItemAdded(const QString& id)
{
	LYXERR(Debug::GUI, "Item added: " << fromqstr(id));
}


void GuiBuddiesWidget::onItemRemoved(const QString& id)
{
	LYXERR(Debug::GUI, "Item removed: " << fromqstr(id));
}


void GuiBuddiesWidget::on_connectPB_clicked()
{
	username = userEdit->text();
	QString title = QString("Enter password") + " for " + username + ": ";
	bool ok;
	password = QInputDialog::getText(this, title, title, QLineEdit::Password, password, &ok);
	if (ok) {
		on_disconnectPB_clicked();
		view_.chatMessenger().qxmpp_client.connectToServer(username, password);
		lyxrc.user_chat_id = fromqstr(username);
		dispatch(FuncRequest(LFUN_PREFERENCES_SAVE));
		storePasswordFor(fromqstr(username), fromqstr(password));
	}
}


void GuiBuddiesWidget::on_disconnectPB_clicked()
{
	view_.chatMessenger().qxmpp_client.disconnectFromServer();
	buddiesList->clear();
	delPB->setEnabled(false);
}


void GuiBuddiesWidget::onStateChanged(QXmppClient::State state)
{
	if (state == QXmppClient::DisconnectedState)
		statusLabel->setText(qt_("Disconnected"));
	else if (state == QXmppClient::ConnectedState)
		statusLabel->setText(qt_("Connected"));
	else if (state == QXmppClient::ConnectingState)
		statusLabel->setText(qt_("Connecting..."));
}


void GuiBuddiesWidget::onStateSelected(const QString & qs)
{
	QXmppPresence p;
	string s = fromqstr(qs);
	if (s == "Available")
		p.setAvailableStatusType(QXmppPresence::Online);
	else if (s == "Away")
		p.setAvailableStatusType(QXmppPresence::Away);
	else if (s == "Do Not Disturb")
		p.setAvailableStatusType(QXmppPresence::DND);
	view_.chatMessenger().qxmpp_client.setClientPresence(p);
}


void GuiBuddiesWidget::onItemDoubleClicked(QListWidgetItem *p_item)
{
	GuiChat *p_dlg = dynamic_cast<GuiChat *>(view_.findOrBuild("chat-bar", false));
	if (p_dlg) {
		GuiChatWidget *p_chat = p_dlg->chatWidget();
		p_chat->setUserName(fromqstr(username));
		string destname = fromqstr(p_item->text());
		p_chat->setDestName(destname);
		p_chat->ensureChatVisible(destname);
		view_.currentWorkArea()->stopBlinkingCursor();
		p_dlg->show();
		p_dlg->setFocus();
	}
}


void GuiBuddiesWidget::onItemClicked(QListWidgetItem *)
{
	delPB->setEnabled(true);
}


void GuiBuddiesWidget::on_addPB_clicked()
{
	bool ok;
	QString peername = QInputDialog::getText(this, QString(), qt_("Username of buddy to add:"), QLineEdit::Normal, QString(), &ok);
	if (ok) {
		view_.chatMessenger().qxmpp_client.rosterManager().subscribe(peername);
		view_.chatMessenger().qxmpp_client.rosterManager().addItem(peername);
	}
}


void GuiBuddiesWidget::on_delPB_clicked()
{
	QListWidgetItem *p_curr_item = buddiesList->currentItem();
	if (p_curr_item != 0) {
		QString peername = p_curr_item->text();
		QMessageBox::StandardButton const answer = QMessageBox::question(
			this,
			qt_("Confirmation dialog"),
			qt_("Do you really want to delete ") + peername + " from your buddies?",
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if (answer == QMessageBox::Yes) {
			view_.chatMessenger().qxmpp_client.rosterManager().removeItem(peername);
			buddiesList->removeItemWidget(p_curr_item);
		}
	}
}


void GuiBuddiesWidget::onSubscriptionReceived(const QString& peername)
{
	QMessageBox::StandardButton const answer = QMessageBox::question(
		this, qt_("Confirmation dialog"),
		qt_("Peer ") + peername + qt_(" is asking permission to subscribe to your on-line status. Would you like to accept?"),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	if (answer == QMessageBox::Yes) {
		view_.chatMessenger().qxmpp_client.rosterManager().acceptSubscription(peername);
		view_.chatMessenger().qxmpp_client.rosterManager().subscribe(peername);
		view_.chatMessenger().qxmpp_client.rosterManager().addItem(peername);
	} else {
		view_.chatMessenger().qxmpp_client.rosterManager().refuseSubscription(peername);
	}
}


GuiBuddies::GuiBuddies(GuiView & parent,
		Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "chat", qt_("LyX Chat"),
		   area, flags)
{
	widget_ = new GuiBuddiesWidget(parent);
	setWidget(widget_);
	setFocusProxy(widget_);
	setVisible(true);
}


GuiBuddies::~GuiBuddies()
{
	setFocusProxy(0);
	delete widget_;
}


bool GuiBuddies::initialiseParams(std::string const & params)
{
	return widget_->initialiseParams(params);
}


Dialog * createGuiBuddies(GuiView & lv)
{
	GuiBuddies * gui = new GuiBuddies(lv, Qt::LeftDockWidgetArea);
	GuiBuddiesWidget *p_buddies = gui->buddiesWidget();
	lv.chatMessenger().p_buddies = p_buddies;

#ifdef Q_WS_MACX
	// On Mac show and floating
	gui->setFloating(true);
#endif
	return gui;
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiBuddies.cpp"
