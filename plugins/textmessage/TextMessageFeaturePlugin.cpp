/*
 * TextMessageFeaturePlugin.cpp - implementation of TextMessageFeaturePlugin class
 *
 * Copyright (c) 2017-2026 Tobias Junghans <tobydox@veyon.io>
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

#include <QMessageBox>

#include "TextMessageFeaturePlugin.h"
#include "TextMessageDialog.h"
#include "StickyMessageWidget.h"
#include "FeatureWorkerManager.h"
#include "ComputerControlInterface.h"
#include "PlatformCoreFunctions.h"
#include "VeyonMasterInterface.h"
#include "VeyonServerInterface.h"


TextMessageFeaturePlugin::TextMessageFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_textMessageFeature( Feature( QStringLiteral( "TextMessage" ),
								   Feature::Flag::Action | Feature::Flag::AllComponents,
								   Feature::Uid( "e75ae9c8-ac17-4d00-8f0d-019348346208" ),
								   Feature::Uid(),
								   tr( "Text message" ), {},
								   tr( "Use this function to send a text message to all "
									   "users e.g. to assign them new tasks." ),
								   QStringLiteral(":/textmessage/dialog-information.png") ) ),
	m_clearAnnouncementFeature( Feature( QStringLiteral( "ClearAnnouncement" ),
										 Feature::Flag::Action | Feature::Flag::AllComponents,
										 Feature::Uid( "7f2a1c4e-3b8d-4e6a-9c1f-2d5b8a0e4f73" ),
										 Feature::Uid(),
										 tr( "Clear announcement" ), {},
										 tr( "Remove the announcement banner from all student screens." ),
										 QStringLiteral(":/textmessage/dialog-information.png") ) ),
	m_features( { m_textMessageFeature, m_clearAnnouncementFeature } )
{
}



TextMessageFeaturePlugin::~TextMessageFeaturePlugin()
{
	delete m_stickyWidget;
}



const FeatureList &TextMessageFeaturePlugin::featureList() const
{
	return m_features;
}



bool TextMessageFeaturePlugin::controlFeature( Feature::Uid featureUid,
											  Operation operation,
											  const QVariantMap& arguments,
											  const ComputerControlInterfaceList& computerControlInterfaces )
{
	if( operation != Operation::Start )
	{
		return false;
	}

	if( featureUid == m_clearAnnouncementFeature.uid() )
	{
		if( operation == Operation::Start )
		{
			sendFeatureMessage( FeatureMessage{ m_textMessageFeature.uid(),
												 FeatureCommand::HideTextMessage },
								computerControlInterfaces );
			return true;
		}
		return false;
	}

	if( featureUid == m_textMessageFeature.uid() )
	{
		const auto text = arguments.value( argToString(Argument::Text) ).toString();
		const auto icon = arguments.value( argToString(Argument::Icon) ).toInt();
		const auto title = arguments.value( argToString(Argument::Title) ).toString();

		sendFeatureMessage(FeatureMessage{featureUid, FeatureCommand::ShowTextMessage}
						   .addArgument(Argument::Text, text)
						   .addArgument(Argument::Icon, icon)
						   .addArgument(Argument::Title, title), computerControlInterfaces);

		return true;
	}

	return false;
}



bool TextMessageFeaturePlugin::startFeature( VeyonMasterInterface& master, const Feature& feature,
											 const ComputerControlInterfaceList& computerControlInterfaces )
{
	if( feature == m_clearAnnouncementFeature )
	{
		controlFeature( m_clearAnnouncementFeature.uid(), Operation::Start,
						{}, computerControlInterfaces );
		return true;
	}

	if( feature.uid() != m_textMessageFeature.uid() )
	{
		return false;
	}

	QString textMessage;
	QString messageTitle;

	TextMessageDialog( textMessage, messageTitle, master.mainWindow() ).exec();

	if( textMessage.isEmpty() == false )
	{
		controlFeature( m_textMessageFeature.uid(), Operation::Start,
						{
							{ argToString(Argument::Text), textMessage },
							{ argToString(Argument::Icon), QMessageBox::Information },
							{ argToString(Argument::Title), messageTitle }
						},
						computerControlInterfaces );
	}

	return true;
}




bool TextMessageFeaturePlugin::handleFeatureMessage( VeyonServerInterface& server,
													 const MessageContext& messageContext,
													 const FeatureMessage& message )
{
	Q_UNUSED(messageContext)

	if( m_textMessageFeature.uid() == message.featureUid() ||
		m_clearAnnouncementFeature.uid() == message.featureUid() )
	{
		// forward message to worker
		server.featureWorkerManager().sendMessageToUnmanagedSessionWorker( message );

		return true;
	}

	return false;
}



bool TextMessageFeaturePlugin::handleFeatureMessage( VeyonWorkerInterface& worker, const FeatureMessage& message )
{
	Q_UNUSED(worker);

	if( message.featureUid() == m_textMessageFeature.uid() ||
		message.featureUid() == m_clearAnnouncementFeature.uid() )
	{
		if( message.command<FeatureCommand>() == FeatureCommand::HideTextMessage )
		{
			delete m_stickyWidget;
			m_stickyWidget = nullptr;
			return true;
		}

		auto title = message.argument( Argument::Title ).toString();
		if( title.isEmpty() )
		{
			title = tr( "Message from teacher" );
		}

		delete m_stickyWidget;
		m_stickyWidget = new StickyMessageWidget( title,
												  message.argument( Argument::Text ).toString() );
		m_stickyWidget->setAttribute( Qt::WA_DeleteOnClose );
		m_stickyWidget->show();

		return true;
	}

	return true;
}
