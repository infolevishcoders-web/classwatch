/*
 * ClassWatchMessagePanel.h - panel for ClassWatch messaging and communication
 *
 * Copyright (c) 2026 Levish Coders
 *
 * This file is part of ClassWatch - https://levishcoders.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#pragma once

#include <QWidget>
#include "ClassWatchMessageManager.h"

namespace Ui {
class ClassWatchMessagePanel;
}

class VeyonMaster;

class ClassWatchMessagePanel : public QWidget
{
	Q_OBJECT

public:
	explicit ClassWatchMessagePanel( VeyonMaster* master, QWidget* parent = nullptr );
	~ClassWatchMessagePanel() override;

private Q_SLOTS:
	void onSendMessage();
	void onClearMessage();
	void onRefreshHistory();
	void onClearHistory();
	void onExportHistory();
	void onBrowseFile();
	void onSendFile();
	void onSendChatMessage();
	void onClose();
	void onMessageStatusChanged( const QString& messageId );
	void onFileTransferProgress( const QString& transferId, int progress );

private:
	void setupConnections();
	void populateRecipientList();
	void populateMessageHistory();
	void populateTransferHistory();
	void populateChatHistory();
	void updateStatus( const QString& status );
	QString getSelectedRecipient() const;
	QString getMessageType() const;
	void refreshAll();

	Ui::ClassWatchMessagePanel* ui;
	VeyonMaster* m_master;
	ClassWatchMessageManager* m_messageManager;
};
