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

#ifndef QGUICHAT_H
#define QGUICHAT_H

#include "GuiWorkArea.h"
#include "GuiChatMessenger.h"

#include "DockView.h"
// This is needed so that ui_ChatUi.h can find qt_()
#include "qt_helpers.h"
#include "ui_ChatUi.h"

#include "BufferView.h"
#include "Buffer.h"
#include "LyX.h"
#include "Text.h"
#include "lyxfind.h"

#include <QDialog>

#include <string>

namespace lyx {
namespace frontend {

class GuiChatWidget;

class GuiChatWidget : public QTabWidget, public Ui::ChatUi
{
	Q_OBJECT

public:
	GuiChatWidget(GuiView & view);
	~GuiChatWidget();
	bool initialiseParams(std::string const & params);
	void onMessageReceived(std::string const & from, std::string const & s);

	void setUserName(std::string const & name);
	void setDestName(std::string const & name);

	void ensureChatVisible(const std::string & peername);

protected:
	///
	GuiView & view_;

	bool eventFilter(QObject *obj, QEvent *event);

	virtual void showEvent(QShowEvent *ev);
	virtual void hideEvent(QHideEvent *ev);

	void hideDialog();

	void appendToChat(std::string const & peername, std::string const & prefix, docstring const & latex);

protected Q_SLOTS:
	void on_sendPB_clicked();
};


class GuiChat : public DockView
{
	Q_OBJECT
public:
	GuiChat(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::BottomDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);

	~GuiChat();

	bool initialiseParams(std::string const &);
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return false; }

	/// update
	void updateView() {}

	GuiChatWidget * chatWidget() { return widget_; }

protected:
	virtual bool wantInitialFocus() const { return true; }

private:
	/// The encapsulated widget.
	GuiChatWidget * widget_;
};


} // namespace frontend
} // namespace lyx

#endif // QGUICHAT_H
