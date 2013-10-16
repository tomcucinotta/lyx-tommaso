/**
 * \file GuiChat.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Tommaso Cucinotta
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiChat.h"
#include "GuiBuddies.h"

#include "CutAndPaste.h"
#include "Lexer.h"
#include "GuiApplication.h"
#include "GuiView.h"
#include "GuiWorkArea.h"
#include "qt_helpers.h"
#include "Language.h"

#include "BufferParams.h"
#include "BufferList.h"
#include "TextClass.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "lyxfind.h"
#include "output_latex.h"
#include "texstream.h"
#include "support/Package.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/TempFile.h"

#include <QCloseEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>

#include <qxmpp/QXmppMessage.h>
#include <qxmpp/QXmppRosterManager.h>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiChatWidget::GuiChatWidget(GuiView & view)
	: QTabWidget(&view), view_(view)
{
	setupUi(this);
	chat_work_area_->setGuiView(view_);
	chat_work_area_->init();
	chat_work_area_->setFrameStyle(QFrame::StyledPanel);

	setFocusProxy(chat_work_area_);
	// We don't want two cursors blinking.
	chat_work_area_->stopBlinkingCursor();
}


GuiChatWidget::~GuiChatWidget() {
}


void GuiChatWidget::setUserName(std::string const & name)
{
	fromLE->setText(toqstr(name));
}


void GuiChatWidget::setDestName(std::string const & name)
{
	destLE->setText(toqstr(name));
}


void GuiChatWidget::onMessageReceived(string const & from, string const & s)
{
	if (s.size() > 0) {
		appendToChat(from, from, from_utf8(s));
		chat_work_area_->setFocus();
	}
}


bool GuiChatWidget::eventFilter(QObject * obj, QEvent * event)
{
	if (event->type() != QEvent::KeyPress
		  || (obj != chat_work_area_))
		return QWidget::eventFilter(obj, event);

	QKeyEvent * e = static_cast<QKeyEvent *> (event);
	switch (e->key()) {
	case Qt::Key_Escape:
		if (e->modifiers() == Qt::NoModifier) {
			hideDialog();
			return true;
		}
		break;

	case Qt::Key_Enter:
	case Qt::Key_Return: {
		on_sendPB_clicked();
		return true;
		}

	default:
		break;
	}
	// standard event processing
	return QWidget::eventFilter(obj, event);
}


void GuiChatWidget::hideDialog()
{
	theGuiApp()->setCurrentView(&view_);
	dispatch(FuncRequest(LFUN_DIALOG_HIDE, "chat"));
}


static docstring buffer_to_latex(Buffer & buffer) 
{
	OutputParams runparams(&buffer.params().encoding());
	odocstringstream ods;
	otexstream os(ods);
	runparams.nice = true;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = 80; //lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;
	pit_type const endpit = buffer.paragraphs().size();
	for (pit_type pit = 0; pit != endpit; ++pit) {
		TeXOnePar(buffer, buffer.text(), pit, os, runparams);
		LYXERR(Debug::GUI, "searchString up to here: " << ods.str());
	}
	return ods.str();
}


void GuiChatWidget::ensureChatVisible(const string & peername)
{
	FileName chat_file = view_.chatMessenger().chatFileName(peername);

	theGuiApp()->setCurrentView(&view_);
	dispatch(FuncRequest(LFUN_FILE_OPEN, chat_file.absFileName()));
	dispatch(FuncRequest(LFUN_BUFFER_END));
}


void GuiChatWidget::appendToChat(const string & destname, string const & prefix, docstring const & latex)
{
	string peername = GuiChatMessenger::cutResource(destname);
	ensureChatVisible(peername);

	BufferView & bv = view_.currentMainWorkArea()->bufferView();
	ErrorList & el = bv.buffer().errorList("Parse");

	theGuiApp()->setCurrentView(&view_);
	dispatch(FuncRequest(LFUN_BUFFER_END));
	dispatch(FuncRequest(LFUN_PARAGRAPH_BREAK));

	support::TempFile tempfile("embedded.internal");
	tempfile.setAutoRemove(false);
	Buffer * new_buf = theBufferList().newInternalBuffer(tempfile.name().absFileName());
	new_buf->setUnnamed(true);
	new_buf->setFullyLoaded(true);

	string color = string("\\textcolor{");
	if (prefix != destname)
		color = color + "blue";
	else
		color = color + "red";
	color = color + "}";
	new_buf->importString("latex", from_ascii(color + "\\textbf{" + prefix + "}: "), el);
	cap::pasteParagraphList(bv.cursor(), new_buf->paragraphs(),
				new_buf->params().documentClassPtr(), el);

	if (!new_buf->importString("latexclipboard", latex, el))
		new_buf->importString("txt", latex, el);
	cap::pasteParagraphList(bv.cursor(), new_buf->paragraphs(),
				new_buf->params().documentClassPtr(), el);
	bv.buffer().changed(true);
	dispatch(FuncRequest(LFUN_BUFFER_WRITE));
	//bv.processUpdateFlags(Update::Force | Update::FitCursor);
}


void GuiChatWidget::on_sendPB_clicked() 
{
	string username = fromqstr(fromLE->text());
	string destname = fromqstr(destLE->text());
	if (username.empty() || destname.empty()) {
		QMessageBox::warning(this, "Incorrect parameters", "Please, check that both the \"From\" and the \"To\" username fields are non-empty!");
		return;
	}
	Buffer & msg_buf = chat_work_area_->bufferView().buffer();
	docstring latex = buffer_to_latex(msg_buf);
	view_.chatMessenger().sendMessage(username, destname, to_utf8(latex));
	// includes theGuiApp()->setCurrentView(&view_);
	appendToChat(destname, username, latex);

	chat_work_area_->setFocus();
	dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
	dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
	dispatch(FuncRequest(LFUN_CUT));
}


void GuiChatWidget::showEvent(QShowEvent * ev)
{
	this->QTabWidget::showEvent(ev);
	LYXERR(Debug::FIND, "showEvent()" << endl);
	chat_work_area_->installEventFilter(this);
	view_.setCurrentWorkArea(chat_work_area_);
	LYXERR(Debug::FIND, "Selecting entire chat buffer");
	DispatchResult dr;
	theGuiApp()->setCurrentView(&view_);
	dispatch(FuncRequest(LFUN_BUFFER_BEGIN), dr);
	dispatch(FuncRequest(LFUN_BUFFER_END_SELECT), dr);
}


void GuiChatWidget::hideEvent(QHideEvent *ev)
{
	LYXERR(Debug::FIND, "hideEvent");
	chat_work_area_->removeEventFilter(this);
	this->QTabWidget::hideEvent(ev);
}


bool GuiChatWidget::initialiseParams(std::string const & /*params*/)
{
	return true;
}


GuiChat::GuiChat(GuiView & parent,
		Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "message", qt_("LyX Chat Message Window"),
		   area, flags)
{
	widget_ = new GuiChatWidget(parent);
	setWidget(widget_);
	setFocusProxy(widget_);
}


GuiChat::~GuiChat()
{
	setFocusProxy(0);
	delete widget_;
}


bool GuiChat::initialiseParams(std::string const & params)
{
	return widget_->initialiseParams(params);
}


Dialog * createGuiChat(GuiView & lv)
{
	GuiChat * gui = new GuiChat(lv, Qt::BottomDockWidgetArea);
	GuiChatWidget *p_chat = gui->chatWidget();
	lv.chatMessenger().p_chat = p_chat;

#ifdef Q_WS_MACX
	// On Mac show and floating
	gui->setFloating(true);
#endif

	return gui;
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiChat.cpp"
