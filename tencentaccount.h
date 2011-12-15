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

#ifndef TENCENTACCOUNT_H
#define TENCENTACCOUNT_H

#include <choqok/account.h>

namespace QOAuth { class Interface; }
class TencentMicroBlog;

class TencentAccount : public Choqok::Account
{
    Q_OBJECT
    public:
        explicit TencentAccount( TencentMicroBlog* parent, const QString& alias );
        virtual ~TencentAccount();
        virtual void writeConfig();

        static const QByteArray oauthConsumerKey();
        static const QByteArray oauthConsumerSecret();

        QOAuth::Interface* qoauthInterface() const;
        void setOauthToken( const QByteArray& token );
        const QByteArray oauthToken() const;
        void setOauthTokenSecret( const QByteArray& tokenSecret );
        const QByteArray oauthTokenSecret() const;
        QString clientip;

        virtual QStringList timelineNames() const;
        void setTimelineNames( const QStringList& list );
    private:
        QOAuth::Interface* qoauth;
        QByteArray m_oauthToken;
        QByteArray m_oauthTokenSecret;
        QStringList m_timelineNames;
};

#endif // TENCENTACCOUNT_H
