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

#ifndef TENCENTMICROBLOG_H
#define TENCENTMICROBLOG_H

#include <choqok/microblog.h>
#include <qjson/parser.h>
#include <QHash>

class KJob;
class TencentAccount;

class TencentMicroBlog : public Choqok::MicroBlog
{
    Q_OBJECT
    public:
        explicit TencentMicroBlog( QObject* parent, const QVariantList& args );
        virtual ~TencentMicroBlog();
        virtual void aboutToUnload();
        virtual ChoqokEditAccountWidget* createEditAccountWidget( Choqok::Account* account, QWidget* parent );
        virtual Choqok::UI::PostWidget* createPostWidget( Choqok::Account* account, const Choqok::Post& post, QWidget* parent );
        virtual void createPost( Choqok::Account* theAccount, Choqok::Post* post );
        virtual void abortCreatePost( Choqok::Account* theAccount, Choqok::Post* post = 0 );
        virtual void fetchPost( Choqok::Account* theAccount, Choqok::Post* post );
        virtual void removePost( Choqok::Account* theAccount, Choqok::Post* post );
        virtual void saveTimeline( Choqok::Account* account, const QString& timelineName,
                                   const QList<Choqok::UI::PostWidget*>& timeline );
        virtual QList<Choqok::Post*> loadTimeline( Choqok::Account* theAccount, const QString& timelineName );
        virtual Choqok::Account* createNewAccount( const QString& alias );
        virtual void updateTimelines( Choqok::Account* theAccount );
        virtual Choqok::TimelineInfo* timelineInfo( const QString& timelineName );
        virtual QString postUrl( Choqok::Account* account, const QString& username, const QString& postId ) const;
        virtual QString profileUrl( Choqok::Account* account, const QString& username ) const;
        void retweetPost( Choqok::Account* theAccount, Choqok::Post* post );
        void createFavorite( Choqok::Account* theAccount, Choqok::Post* post );
        void removeFavorite( Choqok::Account* theAccount, Choqok::Post* post );
        void createFriendship( Choqok::Account* theAccount, Choqok::User* user );
        void removeFriendship( Choqok::Account* theAccount, Choqok::User* user );
    Q_SIGNALS:
        void favoriteCreated( Choqok::Account* account, Choqok::Post* post );
        void favoriteRemoved( Choqok::Account* account, Choqok::Post* post );
        void friendshipCreated( Choqok::Account* theAccount, Choqok::User* user );
        void friendshipRemoved( Choqok::Account* theAccount, Choqok::User* user );
    private Q_SLOTS:
        void slotCreatePost( KJob* job );
        void slotFetchPost( KJob* job );
        void slotRemovePost( KJob* job );
        void slotRequestTimeline( KJob* job );
        void slotCreateFavorite( KJob* job );
        void slotRemoveFavorite( KJob* job );
        void slotCreateFriendship( KJob* job );
        void slotRemoveFriendship( KJob* job );
    private:
        int m_countOfTimelinesToSave;
        QHash<QString, QString> m_timelineApiPath;
        QHash<QString, Choqok::TimelineInfo*> m_timelineInfo;
        QHash<TencentAccount*, QHash<QString, QDateTime> > m_timelineLatestTime;
        QHash<TencentAccount*, QHash<QString, QString> > m_timelineLatestId;
        QHash<KJob*, Choqok::Post*> m_createPost;
        QHash<KJob*, Choqok::Post*> m_fetchPost;
        QHash<KJob*, Choqok::Post*> m_removePost;
        QHash<KJob*, QString> m_jobTimeline;
        QHash<KJob*, Choqok::Post*> m_createFavorite;
        QHash<KJob*, Choqok::Post*> m_removeFavorite;
        QHash<KJob*, Choqok::User*> m_createFriendship;
        QHash<KJob*, Choqok::User*> m_removeFriendship;
        QHash<KJob*, TencentAccount*> m_jobAccount;
        QJson::Parser parser;

        QDateTime dateFromString( const QString &date );
        QHash<QString, int> monthes;

        /// json data
        void readPostFromJsonMap( const QVariantMap& varmap, Choqok::Post* post );
        void readDMessageFromJsonMap( Choqok::Account* account, const QVariantMap& varmap, Choqok::Post* post );
        void readUserFromJsonMap( const QVariantMap& varmap, Choqok::User* user );
};

#endif // NETEASEMICROBLOG_H
