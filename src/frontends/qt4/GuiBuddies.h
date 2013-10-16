// -*- C++ -*-
/**
 * \file GuiBuddies.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Tommaso Cucinotta
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QGUIBUDDIES_H
#define QGUIBUDDIES_H

#include <config.h>
#include <qxmpp/QXmppClient.h>

#include "GuiWorkArea.h"

#include "DockView.h"
// This is needed so that ui_BuddiesUi.h can find qt_()
#include "qt_helpers.h"
#include "ui_BuddiesUi.h"

#include "BufferView.h"
#include "Buffer.h"
#include "LyX.h"
#include "Text.h"
#include "lyxfind.h"

#include <QDialog>
#include <QTabWidget>

#include <string>

class QXmppPresence;

namespace lyx {
namespace frontend {

class GuiBuddiesWidget : public QTabWidget, public Ui::buddiesPane
{
	Q_OBJECT

public:
	GuiBuddiesWidget(GuiView & view);
	~GuiBuddiesWidget();
	bool initialiseParams(std::string const & params);

	QString username;
	QString password;

public Q_SLOTS:
	void onStateChanged(QXmppClient::State state);
	void onStateSelected(const QString &);

private:
	///
	GuiView & view_;
	QIcon icon_avail;
	QIcon icon_offline;
	QIcon icon_away;
	QIcon icon_dnd;

	QIcon presenceToIcon(const QXmppPresence & pres);
	std::string getPasswordFor(const std::string & username);
	void storePasswordFor(const std::string & username, const std::string & pwd);

private Q_SLOTS:
	void on_connectPB_clicked();
	void on_disconnectPB_clicked();
	void on_addPB_clicked();
	void on_delPB_clicked();
	void onItemClicked(QListWidgetItem*);
	void onItemDoubleClicked(QListWidgetItem*);
	void onPresenceChanged(const QString&, const QString&);
	void onItemAdded(const QString&);
	void onItemRemoved(const QString&);
	void onSubscriptionReceived(const QString&);
};


class GuiBuddies : public DockView
{
	Q_OBJECT
public:
	GuiBuddies(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::BottomDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

	~GuiBuddies();

	bool initialiseParams(std::string const &);
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return false; }
	bool needBufferOpen() const { return false; }

	/// update
	void updateView() {}

	GuiBuddiesWidget * buddiesWidget() { return widget_; }

private:
	/// The encapsulated widget.
	GuiBuddiesWidget * widget_;
};


} // namespace frontend
} // namespace lyx

#endif // QGUIBUDDIES_H
