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

#include "tencentmicroblog.h"

#include "tencentaccount.h"
#include "tencenteditaccount.h"
#include "tencentpostwidget.h"

#include <choqok/accountmanager.h>
#include <choqok/choqokbehaviorsettings.h>
#include <choqok/notifymanager.h>
#include <choqok/postwidget.h>
#include <choqok/application.h>
#include <KGenericFactory>
#include <kio/job.h>

#include <QtOAuth/QtOAuth>

static const char apiUrl[] = "http://open.t.qq.com/api";

K_PLUGIN_FACTORY(TencentMicroBlogFactory, registerPlugin<TencentMicroBlog>(); )
K_EXPORT_PLUGIN( TencentMicroBlogFactory( "choqok_tencent" ) )

TencentMicroBlog::TencentMicroBlog( QObject* parent, const QVariantList& args )
: MicroBlog(TencentMicroBlogFactory::componentData(), parent)
{
    Q_UNUSED(args)
    setServiceName( "Tencent" );
    setServiceHomepageUrl( "http://t.qq.com/" );
    setCharLimit( 140 );

    QStringList m_timelineNames;
    m_timelineNames << "home" /* << "favorite"*/ << "public" << "mentions" << "inbox" << "outbox" /*<< "broadcast"*/;
    setTimelineNames( m_timelineNames );

    m_timelineApiPath[ "home" ] = "/statuses/home_timeline";
    m_timelineApiPath[ "public" ] = "/statuses/public_timeline";
    m_timelineApiPath[ "mentions" ] = "/statuses/mentions_timeline";
    m_timelineApiPath[ "inbox" ] = "/private/recv";
    m_timelineApiPath[ "outbox" ] = "/private/send";
    //m_timelineApiPath[ "broadcast" ] = "/statuses/broadcast_timeline";

    m_countOfTimelinesToSave = 0;
    monthes["Jan"] = 1;
    monthes["Feb"] = 2;
    monthes["Mar"] = 3;
    monthes["Apr"] = 4;
    monthes["May"] = 5;
    monthes["Jun"] = 6;
    monthes["Jul"] = 7;
    monthes["Aug"] = 8;
    monthes["Sep"] = 9;
    monthes["Oct"] = 10;
    monthes["Nov"] = 11;
    monthes["Dec"] = 12;

    /// set timeline info
    Choqok::TimelineInfo* info = new Choqok::TimelineInfo;
    info->name = i18nc( "Timeline Name", "Home" );
    info->description = i18nc( "Timeline description", "You and your friends" );
    info->icon = "user-home";
    m_timelineInfo[ "home" ] = info;

    info = new Choqok::TimelineInfo;
    info->name = i18nc( "Timeline Name", "Inbox" );
    info->description = i18nc( "Timeline description", "Your incoming private messages" );
    info->icon = "mail-folder-inbox";
    m_timelineInfo[ "inbox" ] = info;

    info = new Choqok::TimelineInfo;
    info->name = i18nc( "Timeline Name", "Outbox" );
    info->description = i18nc( "Timeline description", "Private messages you have sent" );
    info->icon = "mail-folder-outbox";
    m_timelineInfo[ "outbox" ] = info;

//     info = new Choqok::TimelineInfo;
//     info->name = i18nc( "Timeline Name", "Favorite" );
//     info->description = i18nc( "Timeline description", "Your favorites" );
//     info->icon = "favorites";
//     m_timelineInfo[ "favorite" ] = info;

    info = new Choqok::TimelineInfo;
    info->name = i18nc( "Timeline Name", "Public" );
    info->description = i18nc( "Timeline description", "Public timeline" );
    info->icon = "folder-green";
    m_timelineInfo[ "public" ] = info;

    info = new Choqok::TimelineInfo;
    info->name = i18nc( "Timeline Name", "Mentions" );
    info->description = i18nc( "Timeline description", "Mentions you" );
    info->icon = "edit-redo";
    m_timelineInfo[ "mentions" ] = info;

//    info = new Choqok::TimelineInfo;
 //   info->name = i18nc( "Timeline Name", "User" );
  //  info->description = i18nc( "Timeline description", "Specified user" );
  //  info->icon = "start-here-kde";
  //  m_timelineInfo[ "user" ] = info;

 //   info = new Choqok::TimelineInfo;
  //  info->name = i18nc( "Timeline Name", "ReTweets" );
   // info->description = i18nc( "Timeline description", "ReTweets of me" );
   // info->icon = "folder-red";
   // m_timelineInfo[ "broadcast" ] = info;

//    info = new Choqok::TimelineInfo;
//    info->name = i18nc( "Timeline Name", "Location" );
//    info->description = i18nc( "Timeline description", "Location" );
//    info->icon = "folder-yellow";
//    m_timelineInfo[ "location" ] = info;
}

TencentMicroBlog::~TencentMicroBlog()
{
}

void TencentMicroBlog::aboutToUnload()
{
    m_countOfTimelinesToSave = 0;
    const QList<Choqok::Account*> accounts = Choqok::AccountManager::self()->accounts();
    QList<Choqok::Account*>::ConstIterator it = accounts.constBegin();
    QList<Choqok::Account*>::ConstIterator end = accounts.constEnd();
    while ( it != end ) {
        const Choqok::Account* acc = *it;
        if ( acc->microblog() == this) {
            m_countOfTimelinesToSave += acc->timelineNames().count();
        }
        ++it;
    }
    emit saveTimelines();
}

ChoqokEditAccountWidget* TencentMicroBlog::createEditAccountWidget( Choqok::Account* account, QWidget* parent )
{
    TencentAccount* acc = dynamic_cast<TencentAccount*>(account);
    return new TencentEditAccountWidget( this, acc, parent );
}

Choqok::UI::PostWidget* TencentMicroBlog::createPostWidget( Choqok::Account* account, const Choqok::Post& post, QWidget* parent )
{
    TencentAccount* acc = dynamic_cast<TencentAccount*>(account);
    return new TencentPostWidget( acc, post, parent );
}

void TencentMicroBlog::createPost( Choqok::Account* theAccount, Choqok::Post* post )
{
    if ( !post || post->content.isEmpty() ) {
        qWarning() << "Creating the new post failed. Text is empty.";
        return;
    }

    TencentAccount* acc = dynamic_cast<TencentAccount*>(theAccount);
    QOAuth::ParamMap params;
    if ( post->isPrivate ) {
        /// direct message
        KUrl url( apiUrl );
        url.addPath("/private/add" );

        params.insert("format","json");

        params.insert( "name", post->replyToUserName.toUtf8() );
        params.insert( "content", QUrl::toPercentEncoding( post->content ) );
        params.insert( "clientip", acc->clientip.toUtf8() );
        QOAuth::Interface* qoauth = acc->qoauthInterface();
        QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::POST, acc->oauthToken(), acc->oauthTokenSecret(),QOAuth::HMAC_SHA1, params, QOAuth::ParseForRequestContent );

        KIO::StoredTransferJob* job = KIO::storedHttpPost( hs, url, KIO::HideProgressInfo );
//        job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
 //      job->addMetaData( "customHTTPHeader", "Authorization: " + hs );
        m_createPost[ job ] = post;
        m_jobAccount[ job ] = acc;
        connect( job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)) );
        job->start();
    }
    else {
        /// status update
        KUrl url( apiUrl );

        params.insert("format","json");
        params.insert( "content", QUrl::toPercentEncoding( post->content ) );
        if ( !post->replyToPostId.isEmpty() ) {
            if(post->replyToPostId.startsWith("1_"))
            {
                //reply
                params.insert( "reid", post->replyToPostId.split("_")[1].toUtf8());
                url.addPath("/t/reply");
            }
            else if(post->replyToPostId.startsWith("2_"))
            {
                //rt with comment
                post->postId = post->replyToPostId.split("_")[1];
                retweetPost(acc,post);
                return;
            }
            else
            {
                //direct private
                params.insert( "name", post->replyToPostId.split("_")[1].toUtf8());
                url.addPath("/private/add");

            }
        }
        else
            url.addPath( "/t/add" );

        params.insert( "clientip", acc->clientip.toUtf8() );
        QOAuth::Interface* qoauth = acc->qoauthInterface();
        QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::POST, acc->oauthToken(), acc->oauthTokenSecret(),QOAuth::HMAC_SHA1, params, QOAuth::ParseForRequestContent);

        KIO::StoredTransferJob* job = KIO::storedHttpPost( hs, url, KIO::HideProgressInfo );
        //job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
        //job->addMetaData( "customHTTPHeader", "Authorization: " + hs );
        m_createPost[ job ] = post;
        m_jobAccount[ job ] = acc;
        connect( job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)) );
        job->start();
    }
}

void TencentMicroBlog::abortCreatePost( Choqok::Account* theAccount, Choqok::Post* post )
{
    if ( m_createPost.isEmpty() )
        return;

    if ( !post ) {
        QHash<KJob*, Choqok::Post*>::Iterator it = m_createPost.begin();
        QHash<KJob*, Choqok::Post*>::Iterator end = m_createPost.end();
        while ( it != end ) {
            KJob* job = it.key();
            if ( m_jobAccount.value( job ) == theAccount )
                job->kill( KJob::EmitResult );
            ++it;
        }
    }

    m_createPost.key( post )->kill( KJob::EmitResult );
}

void TencentMicroBlog::fetchPost( Choqok::Account* theAccount, Choqok::Post* post )
{
    if ( !post || post->postId.isEmpty() ) {
        qWarning() << "no id";
        return;
    }

    TencentAccount* acc = dynamic_cast<TencentAccount*>(theAccount);
    KUrl url( apiUrl );
    url.addPath("/t/show");

    QOAuth::ParamMap params;
    params.insert("format","json");
    params.insert("id",post->postId.toUtf8());
    QOAuth::Interface* qoauth = acc->qoauthInterface();
    QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::GET, acc->oauthToken(), acc->oauthTokenSecret(),
                                                    QOAuth::HMAC_SHA1, params, QOAuth::ParseForInlineQuery );
    qDebug()<<"get p:"<<hs;
    url.addPath(hs);
    KIO::StoredTransferJob* job = KIO::storedGet( url, KIO::Reload, KIO::HideProgressInfo );
    m_fetchPost[ job ] = post;
    m_jobAccount[ job ] = acc;
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotFetchPost(KJob*)) );
    job->start();
}

void TencentMicroBlog::removePost( Choqok::Account* theAccount, Choqok::Post* post )
{
    if ( !post || post->postId.isEmpty() ) {
        qWarning() << "Deleting post failed. ID is empty.";
        return;
    }

    TencentAccount* acc = dynamic_cast<TencentAccount*>(theAccount);
    if ( post->isPrivate ) {
        /// direct message
        KUrl url( apiUrl );
        url.addPath( QString( "/private/del" ));

        QOAuth::ParamMap params;
    params.insert("format","json");
    params.insert("id",post->postId.toUtf8());
        QOAuth::Interface* qoauth = acc->qoauthInterface();
        QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::POST, acc->oauthToken(), acc->oauthTokenSecret(),
                                                        QOAuth::HMAC_SHA1, params, QOAuth::ParseForRequestContent );
        KIO::StoredTransferJob* job = KIO::storedHttpPost( hs, url, KIO::HideProgressInfo );
        m_removePost[ job ] = post;
        m_jobAccount[ job ] = acc;
        connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRemovePost(KJob*)) );
        job->start();
    }
    else {
        /// status
        KUrl url( apiUrl );
        url.addPath( QString( "/t/del" ) );

        QOAuth::ParamMap params;
        params.insert("format","json");
        params.insert("id",post->postId.toUtf8());
        QOAuth::Interface* qoauth = acc->qoauthInterface();
        QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::POST, acc->oauthToken(), acc->oauthTokenSecret(),
                                                        QOAuth::HMAC_SHA1, params, QOAuth::ParseForRequestContent );
        KIO::StoredTransferJob* job = KIO::storedHttpPost( hs, url, KIO::HideProgressInfo );
        m_removePost[ job ] = post;
        m_jobAccount[ job ] = acc;
        connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRemovePost(KJob*)) );
        job->start();
    }
}

void TencentMicroBlog::saveTimeline( Choqok::Account* account, const QString& timelineName,
                                     const QList<Choqok::UI::PostWidget*>& timeline )
{
    if(timelineName!="public")//will not save public one
    {
        QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
        KConfig postsBackup( "choqok/" + fileName, KConfig::NoGlobals, "data" );

        ///Clear previous data:
        QStringList prevList = postsBackup.groupList();
        int c = prevList.count();
        if ( c > 0 ) {
            for ( int i = 0; i < c; ++i ) {
                postsBackup.deleteGroup( prevList[i] );
            }
        }
        QList<Choqok::UI::PostWidget*>::ConstIterator it = timeline.constBegin();
        QList<Choqok::UI::PostWidget*>::ConstIterator end = timeline.constEnd();
        while ( it != end ) {
            const Choqok::Post *post = &((*it)->currentPost());
            KConfigGroup grp( &postsBackup, post->creationDateTime.toString() );
            grp.writeEntry( "creationDateTime", post->creationDateTime );
            grp.writeEntry( "postId", post->postId.toString() );
            grp.writeEntry( "text", post->content );
            grp.writeEntry( "source", post->source );
            grp.writeEntry( "inReplyToPostId", post->replyToPostId.toString() );
            grp.writeEntry( "inReplyToUserId", post->replyToUserId.toString() );
            grp.writeEntry( "favorited", post->isFavorited );
            grp.writeEntry( "inReplyToUserName", post->replyToUserName );
            grp.writeEntry( "authorId", post->author.userId.toString() );
            grp.writeEntry( "authorUserName", post->author.userName );
            grp.writeEntry( "authorRealName", post->author.realName );
            grp.writeEntry( "authorProfileImageUrl", post->author.profileImageUrl );
            grp.writeEntry( "authorDescription" , post->author.description );
            grp.writeEntry( "isPrivate" , post->isPrivate );
            grp.writeEntry( "authorLocation" , post->author.location );
            grp.writeEntry( "isProtected" , post->author.isProtected );
            grp.writeEntry( "authorUrl" , post->author.homePageUrl );
            grp.writeEntry( "isRead" , post->isRead );
            grp.writeEntry( "repeatedFrom", post->repeatedFromUsername);
            grp.writeEntry( "repeatedPostId", post->repeatedPostId.toString());
            ++it;
        }
        postsBackup.sync();
    }
    else
        kDebug()<<"we donot save public timeline";
    --m_countOfTimelinesToSave;
    if(Choqok::Application::isShuttingDown()&&m_countOfTimelinesToSave<0)
        emit readyForUnload();
}

QList<Choqok::Post*> TencentMicroBlog::loadTimeline( Choqok::Account* theAccount, const QString& timelineName )
{
    QList<Choqok::Post*> list;

    TencentAccount* account = dynamic_cast<TencentAccount*>(theAccount);

    QString fileName = Choqok::AccountManager::generatePostBackupFileName(account->alias(), timelineName);
    KConfig postsBackup("choqok/" + fileName, KConfig::NoGlobals, "data");
    QStringList tmpList = postsBackup.groupList();
/// to don't load old archives
    if (tmpList.isEmpty() || !(QDateTime::fromString(tmpList.first()).isValid()))
        return list;
///--------------

    QList<QDateTime> groupList;
    foreach(const QString &str, tmpList)
        groupList.append(QDateTime::fromString(str) );
    qSort(groupList);
    int count = groupList.count();
    if( !count )
        return list;

    Choqok::Post* st = 0;
    for ( int i = 0; i < count; ++i ) {
        st = new Choqok::Post;
        KConfigGroup grp( &postsBackup, groupList[i].toString() );
        st->creationDateTime = grp.readEntry( "creationDateTime", QDateTime::currentDateTime() );
        st->postId = grp.readEntry( "postId", QString() );
        st->content = grp.readEntry( "text", QString() );
        st->source = grp.readEntry( "source", QString() );
        st->replyToPostId = grp.readEntry( "inReplyToPostId", QString() );
        st->replyToUserId = grp.readEntry( "inReplyToUserId", QString() );
        st->isFavorited = grp.readEntry( "favorited", false );
        st->replyToUserName = grp.readEntry( "inReplyToUserName", QString() );
        st->author.userId = grp.readEntry( "authorId", QString() );
        st->author.userName = grp.readEntry( "authorUserName", QString() );
        st->author.realName = grp.readEntry( "authorRealName", QString() );
        st->author.profileImageUrl = grp.readEntry( "authorProfileImageUrl", QString() );
        st->author.description = grp.readEntry( "authorDescription" , QString() );
        st->author.isProtected = grp.readEntry("isProtected", false);
        st->isPrivate = grp.readEntry( "isPrivate" , false );
        st->author.location = grp.readEntry("authorLocation", QString());
        st->author.homePageUrl = grp.readEntry("authorUrl", QString());
        st->link = postUrl( account, st->author.userName, st->postId);
        st->isRead = grp.readEntry("isRead", true);
        st->repeatedFromUsername = grp.readEntry("repeatedFrom", QString());
        st->repeatedPostId = grp.readEntry("repeatedPostId", QString());

        list.append( st );
    }

    if ( st )
        m_timelineLatestTime[ account ][ timelineName ] = st->creationDateTime;

    return list;
}

Choqok::Account* TencentMicroBlog::createNewAccount( const QString& alias )
{
    TencentAccount* acc = dynamic_cast<TencentAccount*>(Choqok::AccountManager::self()->findAccount( alias ));
    if ( !acc )
        return new TencentAccount( this, alias );
    else
        return 0;
}

void TencentMicroBlog::updateTimelines( Choqok::Account* theAccount )
{
    TencentAccount* acc = dynamic_cast<TencentAccount*>(theAccount);
    if ( !acc )
        return;

    int countOfPost = Choqok::BehaviorSettings::countOfPosts();

    QHash<QString, QString>::ConstIterator it = m_timelineApiPath.constBegin();
    QHash<QString, QString>::ConstIterator end = m_timelineApiPath.constEnd();
    while ( it != end ) {
        KUrl url( apiUrl );
        url.addPath( it.value() );
        QString timelineName = it.key();
            QDateTime latestStatusId = m_timelineLatestTime[ acc ][ timelineName ];

            QOAuth::ParamMap params;
            if ( latestStatusId.isValid() ) {
               params.insert("pagetime", QString::number(latestStatusId.toTime_t()).toUtf8());
             //   countOfPost = 200;
            }
            else
                params.insert("pagetime","0");
        if(timelineName=="inbox"||timelineName=="outbox")
        {
             params.insert("lastid", "0");

        }
        params.insert( "reqnum", QByteArray::number( countOfPost ) );
        params.insert("format","json");
        params.insert("pageflag","0");

        QOAuth::Interface* qoauth = acc->qoauthInterface();
        QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::GET, acc->oauthToken(), acc->oauthTokenSecret(),
                                                        QOAuth::HMAC_SHA1, params, QOAuth::ParseForInlineQuery);
      //  job->addMetaData( "customHTTPHeader", "Authorization: " + hs );
        url.setQuery(hs);
        kDebug()<<"name:"<<timelineName<<"inbox:"<<url.prettyUrl();
        KIO::StoredTransferJob* job = KIO::storedGet(url , KIO::Reload, KIO::HideProgressInfo );
        m_jobTimeline[job] = timelineName;
        m_jobAccount[job] = acc;
        connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRequestTimeline(KJob*)) );
        job->start();

        ++it;
    }
}

Choqok::TimelineInfo* TencentMicroBlog::timelineInfo( const QString& timelineName )
{
    if ( isValidTimeline( timelineName ) )
        return m_timelineInfo.value( timelineName );
    else
        return 0;
}

QString TencentMicroBlog::postUrl( Choqok::Account* account, const QString& username, const QString& postId ) const
{
    Q_UNUSED(account)
    return QString( "http://t.qq.com/p/t/%1" ).arg( postId );
}

QString TencentMicroBlog::profileUrl( Choqok::Account* account, const QString& username ) const
{
    Q_UNUSED(account)
    return QString( "http://t.qq.com/%1" ).arg( username );
}

void TencentMicroBlog::retweetPost( Choqok::Account* theAccount, Choqok::Post* post )
{
    if ( !post || post->postId.isEmpty() ) {
        qWarning() << "Retweeting post failed. ID is empty.";
        return;
    }

    TencentAccount* acc = dynamic_cast<TencentAccount*>(theAccount);

    KUrl url( apiUrl );
    url.addPath( "/t/re_add");

    QOAuth::ParamMap params;
    params.insert("reid", post->postId.toUtf8() );
    params.insert("content",QUrl::toPercentEncoding( post->content ));
    params.insert("clientip","127.0.0.1");
    params.insert("format","json");
    QOAuth::Interface* qoauth = acc->qoauthInterface();
    QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::POST, acc->oauthToken(), acc->oauthTokenSecret(),
                                                    QOAuth::HMAC_SHA1, params, QOAuth::ParseForRequestContent);
    kDebug()<<"rt:"<<hs;
    KIO::StoredTransferJob* job = KIO::storedHttpPost( hs, url, KIO::HideProgressInfo );
    //job->addMetaData( "customHTTPHeader", "Authorization: " + hs );
    m_createPost[ job ] = post;
    m_jobAccount[ job ] = acc;
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotCreatePost(KJob*)) );
    job->start();
}

void TencentMicroBlog::createFavorite( Choqok::Account* theAccount, Choqok::Post* post )
{
    if ( !post || post->postId.isEmpty() ) {
        qWarning() << "Creating favorite failed. ID is empty.";
        return;
    }

    TencentAccount* acc = dynamic_cast<TencentAccount*>(theAccount);

    KUrl url( apiUrl );
    url.addPath("/fav/addt");

    QOAuth::ParamMap params;
    params.insert("format","json");
    params.insert( "id", post->postId.toUtf8() );
    QOAuth::Interface* qoauth = acc->qoauthInterface();
    QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::POST, acc->oauthToken(), acc->oauthTokenSecret(),
                                                    QOAuth::HMAC_SHA1, params, QOAuth::ParseForRequestContent );
    KIO::StoredTransferJob* job = KIO::storedHttpPost( hs, url, KIO::HideProgressInfo );
    m_createFavorite[ job ] = post;
    m_jobAccount[ job ] = acc;
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotCreateFavorite(KJob*)) );
    job->start();
}

void TencentMicroBlog::removeFavorite( Choqok::Account* theAccount, Choqok::Post* post )
{
    if ( !post || post->postId.isEmpty() ) {
        qWarning() << "del favorite failed. ID is empty.";
        return;
    }

    TencentAccount* acc = dynamic_cast<TencentAccount*>(theAccount);

    KUrl url( apiUrl );
    url.addPath("/fav/delt");

    QOAuth::ParamMap params;
    params.insert("format","json");
    params.insert( "id", post->postId.toUtf8() );
    QOAuth::Interface* qoauth = acc->qoauthInterface();
    QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::POST, acc->oauthToken(), acc->oauthTokenSecret(),
                                                    QOAuth::HMAC_SHA1, params, QOAuth::ParseForRequestContent);
    KIO::StoredTransferJob* job = KIO::storedHttpPost( hs, url, KIO::HideProgressInfo );
    m_removeFavorite[ job ] = post;
    m_jobAccount[ job ] = acc;
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRemoveFavorite(KJob*)) );
    job->start();
}

void TencentMicroBlog::createFriendship( Choqok::Account* theAccount, Choqok::User* user )
{
    if ( !user || ( user->userId.isEmpty() && user->userName.isEmpty() ) ) {
        qWarning() << "Creating friendship failed. ID or username is empty.";
        return;
    }

    TencentAccount* acc = dynamic_cast<TencentAccount*>(theAccount);

    KUrl url( apiUrl );
    url.addPath( "/friend/add" );

    QOAuth::ParamMap params;
    params.insert( "name", user->userName.toUtf8() );
    params.insert("formal","json");
    QOAuth::Interface* qoauth = acc->qoauthInterface();
    QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::POST, acc->oauthToken(), acc->oauthTokenSecret(),
                                                    QOAuth::HMAC_SHA1, params, QOAuth::ParseForRequestContent );

    KIO::StoredTransferJob* job = KIO::storedHttpPost( hs, url, KIO::HideProgressInfo );
    m_createFriendship[ job ] = user;
    m_jobAccount[ job ] = acc;
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotCreateFriendship(KJob*)) );
    job->start();
}

void TencentMicroBlog::removeFriendship( Choqok::Account* theAccount, Choqok::User* user )
{
    if ( !user || ( user->userId.isEmpty() && user->userName.isEmpty() ) ) {
        qWarning() << "Removing friendship failed. ID or username is empty.";
        return;
    }

    TencentAccount* acc = dynamic_cast<TencentAccount*>(theAccount);

    KUrl url( apiUrl );
    url.addPath( "/friends/del" );

    QOAuth::ParamMap params;
    params.insert( "name", user->userName.toUtf8() );
    params.insert("formal","json");
    QOAuth::Interface* qoauth = acc->qoauthInterface();
    QByteArray hs = qoauth->createParametersString( url.url(), QOAuth::POST, acc->oauthToken(), acc->oauthTokenSecret(),
                                                    QOAuth::HMAC_SHA1, params, QOAuth::ParseForRequestContent );

    KIO::StoredTransferJob* job = KIO::storedHttpPost( hs, url, KIO::HideProgressInfo );
    m_removeFriendship[ job ] = user;
    m_jobAccount[ job ] = acc;
    connect( job, SIGNAL(result(KJob*)), this, SLOT(slotRemoveFriendship(KJob*)) );
    job->start();
}

void TencentMicroBlog::slotCreatePost( KJob* job )
{
    if ( job->error() ) {
        qWarning() << "Job Error: " << job->errorString();
        return;
    }

    TencentAccount* acc = m_jobAccount.take( job );
    Choqok::Post* post = m_createPost.take( job );

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);

    if ( post->isPrivate ) {
        /// direct message
        Choqok::NotifyManager::success( i18n( "Private message sent successfully" ) );
    }
    else {
        /// status update
        bool ok;
        QVariantMap varmap = parser.parse( j->data(), &ok ).toMap();
        if ( !ok ) {
            qWarning() << "JSON parsing error.";
            emit errorPost( acc, post, Choqok::MicroBlog::ParsingError,
                            i18n( "Could not parse the data that has been received from the server." ) );
            return;
        }
        kDebug()<<j->data();

        //readPostFromJsonMap( varmap, post );
        if(varmap["ret"].toInt()==0)
            Choqok::NotifyManager::success( i18n( "New post submitted successfully" ) );
        else
            Choqok::NotifyManager::success( "res:"+varmap["msg"].toString());
    }

    emit postCreated( acc, post );
}

void TencentMicroBlog::slotFetchPost( KJob* job )
{
    if ( job->error() ) {
        qWarning() << "Job Error: " << job->errorString();
        return;
    }

    TencentAccount* acc = m_jobAccount.take( job );
    Choqok::Post* post = m_fetchPost.take( job );

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);
    bool ok;
    QVariantMap varmap = parser.parse( j->data(), &ok ).toMap();
    if ( !ok ) {
        qWarning() << "JSON parsing error.";
        emit errorPost( acc, post, Choqok::MicroBlog::ParsingError,
                        i18n( "Could not parse the data that has been received from the server." ) );
        return;
    }
    readPostFromJsonMap( varmap, post );

    emit postFetched( acc, post );
}

void TencentMicroBlog::slotRemovePost( KJob* job )
{
    if ( job->error() ) {
        qWarning() << "Job Error: " << job->errorString();
        return;
    }

    Choqok::Post* post = m_removePost.take( job );
    TencentAccount* acc = m_jobAccount.take( job );

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);
    qWarning() << QString::fromUtf8( j->data() );

    emit postRemoved( acc, post );
}

void TencentMicroBlog::slotRequestTimeline( KJob* job )
{
    if ( job->error() ) {
        qWarning() << "reqtl Job Error: " << job->errorString();
        return;
    }

    TencentAccount* acc = m_jobAccount.take( job );
    QString timelineName = m_jobTimeline.take( job );
    if ( !isValidTimeline( timelineName ) )
        return;

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);

    QList<Choqok::Post*> postlist;

    bool ok;
    QVariantMap res = parser.parse( j->data(), &ok ).toMap();
    if ( !ok ) {
        qWarning() << "slotRequestTimeline JSON parsing failed.";
        return;
    }
    QVariantList postList = res["data"].toMap()["info"].toList();
    QVariantList::ConstIterator it = postList.constBegin();
    QVariantList::ConstIterator end = postList.constEnd();

/*    if ( timelineName == "inbox" || timelineName == "outbox" ) {
        while ( it != end ) {
            QVariantMap varmap = it->toMap();
            Choqok::Post* post = new Choqok::Post;

            readDMessageFromJsonMap( acc, varmap, post );
            postlist.prepend( post );
            ++it;
        }
    }
    else {*/
        while ( it != end ) {
            QVariantMap varmap = it->toMap();
            Choqok::Post* post = new Choqok::Post;
            if ( timelineName == "inbox" || timelineName == "outbox" )
                post->isPrivate=true;
            else
                post->isPrivate=false;
            readPostFromJsonMap( varmap, post );
            postlist.prepend( post );
            ++it;
        }
    //}

    if ( !postlist.isEmpty() )
        m_timelineLatestTime[ acc ][ timelineName ] = QDateTime::fromTime_t(res["data"].toMap()["timestamp"].toInt());
    emit timelineDataReceived( acc, timelineName, postlist );
}

void TencentMicroBlog::slotCreateFavorite( KJob* job )
{
    if ( job->error() ) {
        qWarning() << "Job Error: " << job->errorString();
        return;
    }

    TencentAccount* acc = m_jobAccount.take( job );
    Choqok::Post* post = m_createFavorite.take( job );

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);
    qWarning() << QString::fromUtf8( j->data() );

    emit favoriteCreated( acc, post );
}

void TencentMicroBlog::slotRemoveFavorite( KJob* job )
{
    if ( job->error() ) {
        qWarning() << "Job Error: " << job->errorString();
        return;
    }

    TencentAccount* acc = m_jobAccount.take( job );
    Choqok::Post* post = m_removeFavorite.take( job );

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);
    qWarning() << QString::fromUtf8( j->data() );

    emit favoriteRemoved( acc, post );
}

void TencentMicroBlog::slotCreateFriendship( KJob* job )
{
    if ( job->error() ) {
        qWarning() << "Job Error: " << job->errorString();
        return;
    }

    TencentAccount* acc = m_jobAccount.take( job );
    Choqok::User* user = m_createFriendship.take( job );

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);

    bool ok;
    QVariantMap varmap = parser.parse( j->data(), &ok ).toMap();
    if ( !ok ) {
        qWarning() << "JSON parsing error in slotCreateFriendship.";
        return;
    }
    readUserFromJsonMap( varmap, user );

    emit friendshipCreated( acc, user );
}

void TencentMicroBlog::slotRemoveFriendship( KJob* job )
{
    if ( job->error() ) {
        qWarning() << "Job Error: " << job->errorString();
        return;
    }

    TencentAccount* acc = m_jobAccount.take( job );
    Choqok::User* user = m_removeFriendship.take( job );

    KIO::StoredTransferJob* j = static_cast<KIO::StoredTransferJob*>(job);

    bool ok;
    QVariantMap varmap = parser.parse( j->data(), &ok ).toMap();
    if ( !ok ) {
        qWarning() << "JSON parsing error in slotRemoveFriendship.";
        return;
    }
    readUserFromJsonMap( varmap, user );

    emit friendshipRemoved( acc, user );
}

QDateTime TencentMicroBlog::dateFromString( const QString &date )
{
    char s[10];
    int year, day, hours, minutes, seconds;
    sscanf( qPrintable ( date ), "%*s %s %d %d:%d:%d %*s %d", s, &day, &hours, &minutes, &seconds, &year );
    int month = monthes[s];
    QDateTime recognized ( QDate ( year, month, day ), QTime ( hours, minutes, seconds ) );
    recognized.setTimeSpec( Qt::UTC );
    return recognized.toLocalTime();
}

void TencentMicroBlog::readPostFromJsonMap( const QVariantMap& varmap, Choqok::Post* post )
{
    post->postId = varmap["id"].toString();
    post->source = varmap["from"].toString();
    post->link = varmap["image"].toString();
    post->author.userName = varmap["name"].toString();
    post->author.location = varmap["location"].toString();
    post->author.userId = varmap["uid"].toString();
    post->author.description = "nodesc";
    post->author.realName = varmap["nick"].toString();
    post->author.followersCount = 0;
    if(varmap["head"].toString().length()>1)
        post->author.profileImageUrl = varmap["head"].toString().append("/50");
    else
        post->author.profileImageUrl = "http://mat1.gtimg.com/www/mb/images/head_50.jpg";
    post->author.homePageUrl = QString( "http://t.qq.com/%1" ).arg(post->author.userName);
    post->content = varmap["text"].toString();
    if(varmap["image"].toString()!="null")
    {
        QVariantList img=varmap["image"].toList();
        if(img.length()>0)
        {
            QVariantList::ConstIterator it = img.constBegin();
            post->content.append("\n"+it->toString().append("/160"));
        }
    }
    int type = varmap["type"].toInt();
    if(type!=1)
    {
        if(type!=2)
        {
            post->content.prepend("RT: ");
            post->content.append("\n"+varmap["origtext"].toString());
        }else
        {
            if(varmap["source"].toString()!="null")
            {
                QVariantMap source = varmap["source"].toMap();
                post->content.append("@"+source["name"].toString()+"\n");

                post->content.append(source["text"].toString());
                post->content.append("\n");
//                post->content.append(source["origtext"].toString());
                if(source["image"].toString()!="null")
                {
                    QVariantList img=source["image"].toList();
                    if(img.length()>0)
                    {
                        QVariantList::ConstIterator it = img.constBegin();
                        post->content.append("\n"+it->toString().append("/160"));
                    }
                }
            }

        }
    }
    post->content.replace(QRegExp("<[^>]*>"),"");
    post->creationDateTime = QDateTime::fromTime_t( varmap["timestamp"].toInt() );
    //post->replyToPostId = "0";//varmap["in_reply_to_status_id"].toString();
    //post->replyToUserId = "0";//varmap["in_reply_to_user_id"].toString();
    post->replyToUserName = varmap["name"].toString();
    post->isFavorited = false;//varmap["favorited"].toBool();
}

void TencentMicroBlog::readDMessageFromJsonMap( Choqok::Account* account, const QVariantMap& varmap, Choqok::Post* post )
{
    post->isPrivate = true;
    QString senderId = varmap["sender_id"].toString();
    QString senderScreenName = varmap["sender_screen_name"].toString();

    post->postId = varmap["id"].toString();
    QVariantMap sendermap = varmap["sender"].toMap();
    QVariantMap recipientmap = varmap["recipient"].toMap();
    if ( senderScreenName.compare( account->username(), Qt::CaseInsensitive ) == 0 ) {
        post->author.realName = recipientmap["name"].toString();
        post->author.location = recipientmap["location"].toString();
        post->author.userId = recipientmap["id"].toString();
        post->author.description = recipientmap["description"].toString();
        post->author.userName = recipientmap["screen_name"].toString();
        post->author.followersCount = recipientmap["followers_count"].toInt();
        post->author.profileImageUrl = recipientmap["profile_image_url"].toString();
        post->author.homePageUrl = recipientmap["url"].toString();
        post->isRead = true;
    }
    else {
        post->author.realName = sendermap["name"].toString();
        post->author.location = sendermap["location"].toString();
        post->author.userId = sendermap["id"].toString();
        post->author.description = sendermap["description"].toString();
        post->author.userName = sendermap["screen_name"].toString();
        post->author.followersCount = sendermap["followers_count"].toInt();
        post->author.profileImageUrl = sendermap["profile_image_url"].toString();
        post->author.homePageUrl = sendermap["url"].toString();
    }
    post->content = varmap["text"].toString();
    post->creationDateTime = dateFromString( varmap["created_at"].toString() );
    post->replyToUserId = varmap["recipient_id"].toString();
    post->replyToUserName = varmap["recipient_screen_name"].toString();
}

void TencentMicroBlog::readUserFromJsonMap( const QVariantMap& varmap, Choqok::User* user )
{
    user->realName = varmap["name"].toString();
    user->location = varmap["location"].toString();
    user->userId = varmap["id"].toString();
    user->description = varmap["description"].toString();
    user->userName = varmap["screen_name"].toString();
    user->followersCount = varmap["followers_count"].toInt();
    user->profileImageUrl = varmap["profile_image_url"].toString();
    user->homePageUrl = varmap["url"].toString();
}
