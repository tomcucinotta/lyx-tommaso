// -*- C++ -*-
/**
 * \file DialogView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIALOGVIEW_H
#define DIALOGVIEW_H

#include "Dialog.h"
#include "GuiView.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class DialogView : public QDialog, public Dialog
{
	Q_OBJECT

public:
	/// \param lv is the access point for the dialog to the LyX kernel.
	/// \param name is the identifier given to the dialog by its parent
	/// container.
	/// \param title is the window title used for decoration.
	DialogView(GuiView & lv, QString const & name, QString const & title);

	virtual QWidget * asQWidget() { return this; }
	virtual QWidget const * asQWidget() const { return this; }

protected:
	/// \name Dialog inherited methods
	//@{
	void applyView() {}
	bool initialiseParams(std::string const & /*data*/) { return true; }
	void clearParams() {}
	bool needBufferOpen() const { return isBufferDependent(); }
	//@}
	/// Any dialog that overrides this method should make sure to call it.
	void closeEvent(QCloseEvent * ev);
	/// Any dialog that overrides this method should make sure to call it.
	void hideEvent(QHideEvent * ev);

protected Q_SLOTS:
	void on_bufferViewChanged() {};
};

} // namespace frontend
} // namespace lyx

#endif // DIALOGVIEW_H
