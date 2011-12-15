/*
 *  This file is part of choqok-tencent
 *  Copyright (C) 2011 Ni Hui <shuizhuyuanluo@126.com>
 *  Copyright (C) 2011 Oasis Li <oasis.szli@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tencentaccount.h"

#include "tencentmicroblog.h"

#include <choqok/passwordmanager.h>
#include <KIO/AccessManager>
#include <QtOAuth/QtOAuth>

static const char tencentConsumerKey[] = "f2d56411e41e468ebcba8a52ddec45a4";
static const char tencentConsumerSecret[] = "f1e645c913e0b3f68ea376d6f35a6a36";

TencentAccount::TencentAccount( TencentMicroBlog* parent, const QString& alias )
: Choqok::Account(parent,alias)
{
    m_oauthToken = configGroup()->readEntry( QString( "%1_OAuthToken" ).arg( alias ), QByteArray() );
    m_oauthTokenSecret = Choqok::PasswordManager::self()->readPassword( QString( "%1_OAuthTokenSecret" ).arg( alias ) ).toUtf8();
    m_timelineNames = configGroup()->readEntry( QString( "%1_Timelines" ).arg( alias ), QStringList() );
    clientip = "127.0.0.1";

    qoauth = new QOAuth::Interface( new KIO::AccessManager( this ), this );
    qoauth->setConsumerKey( TencentAccount::oauthConsumerKey() );
    qoauth->setConsumerSecret( TencentAccount::oauthConsumerSecret() );
    qoauth->setRequestTimeout( 10000 );
    qoauth->setIgnoreSslErrors( true );
}

TencentAccount::~TencentAccount()
{
    delete qoauth;
}

void TencentAccount::writeConfig()
{
    configGroup()->writeEntry( QString( "%1_OAuthToken" ).arg( alias() ), m_oauthToken );
    Choqok::PasswordManager::self()->writePassword( QString( "%1_OAuthTokenSecret" ).arg( alias() ),
                                                    QString::fromUtf8( m_oauthTokenSecret ) );
    configGroup()->writeEntry( QString( "%1_Timelines" ).arg( alias() ), m_timelineNames );
    Choqok::Account::writeConfig();
}

const QByteArray TencentAccount::oauthConsumerKey()
{
    return tencentConsumerKey;
}

const QByteArray TencentAccount::oauthConsumerSecret()
{
    return tencentConsumerSecret;
}

QOAuth::Interface* TencentAccount::qoauthInterface() const
{
    return qoauth;
}

void TencentAccount::setOauthToken( const QByteArray& token )
{
    m_oauthToken = token;
}

const QByteArray TencentAccount::oauthToken() const
{
    return m_oauthToken;
}

void TencentAccount::setOauthTokenSecret( const QByteArray& tokenSecret )
{
    m_oauthTokenSecret = tokenSecret;
}

const QByteArray TencentAccount::oauthTokenSecret() const
{
    return m_oauthTokenSecret;
}

QStringList TencentAccount::timelineNames() const
{
    return m_timelineNames;
}

void TencentAccount::setTimelineNames( const QStringList& list )
{
    m_timelineNames.clear();
    foreach ( const QString& name, list ) {
        if ( microblog()->timelineNames().contains( name ) )
            m_timelineNames << name;
    }
}
