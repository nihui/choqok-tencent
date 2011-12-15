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

#include "tencentpostwidget.h"
#include "tencentaccount.h"
#include "tencentmicroblog.h"

#include <choqok/mediamanager.h>

#include <KAction>
#include <KLocale>
#include <KMenu>
#include <KPushButton>
#include <KDebug>

TencentPostWidget::TencentPostWidget( TencentAccount* account, const Choqok::Post& post, QWidget* parent )
: Choqok::UI::PostWidget(account,post,parent)
{
    TencentMicroBlog* microblog = dynamic_cast<TencentMicroBlog*>(account->microblog());
    connect( microblog, SIGNAL(favoriteRemoved(Choqok::Account*,Choqok::Post*)),
             this, SLOT(slotFavoriteRemoved(Choqok::Account*,Choqok::Post*)) );
    connect( microblog, SIGNAL(favoriteCreated(Choqok::Account*,Choqok::Post*)),
             this, SLOT(slotFavoriteCreated(Choqok::Account*,Choqok::Post*)) );
    unFavIcon=KIcon(Choqok::MediaManager::convertToGrayScale(KIcon("rating").pixmap(16)));


}

TencentPostWidget::~TencentPostWidget()
{
}


void TencentPostWidget::initUi()
{
    Choqok::UI::PostWidget::initUi();

    KPushButton* replyButton = addButton( "replyButton", i18n( "Reply" ), "edit-undo" );
    connect( replyButton, SIGNAL(clicked(bool)), this, SLOT(slotReply()) );

    KMenu* menu = new KMenu;
    KAction* replyAction = new KAction( KIcon( "edit-undo" ), i18n( "Reply to %1", currentPost().author.userName ), menu );
    connect( replyAction, SIGNAL(triggered(bool)), this, SLOT(slotReply()) );
    menu->addAction( replyAction );
    KAction* writeAction = new KAction( KIcon( "document-edit" ), i18n( "Write to %1", currentPost().author.userName ), menu );
    connect( writeAction, SIGNAL(triggered(bool)), this, SLOT(slotWrite()) );
    menu->addAction( writeAction );
    if( !currentPost().isPrivate ) {
        KAction* replyAllAction = new KAction( i18n( "Reply to all" ), menu );
        connect( replyAllAction, SIGNAL(triggered(bool)), this, SLOT(slotReplyAll()) );
        menu->addAction(replyAllAction);
    }

    menu->setDefaultAction( replyAction );
    replyButton->setDelayedMenu( menu );

    if( !currentPost().isPrivate ) {
        favoriteButton = addButton( "favoriteButton",i18n( "Favorite" ), "rating" );
        favoriteButton->setCheckable( true );
        connect( favoriteButton, SIGNAL(clicked(bool)), this, SLOT(slotFavorite()) );
        if ( currentPost().isFavorited ) {
            favoriteButton->setChecked( true );
            favoriteButton->setIcon( KIcon( "rating" ) );
        }
        else {
            favoriteButton->setChecked( false );
            favoriteButton->setIcon( unFavIcon);
        }
    }
}

void TencentPostWidget::slotResendPost()
{
//    setReadInternal();
    //TencentMicroBlog* microblog = dynamic_cast<TencentMicroBlog*>(currentAccount()->microblog());
    //Choqok::Post* post = new Choqok::Post;
    //post->postId = currentPost().postId;
    //microblog->retweetPost( currentAccount(), post );
    //QString txt = QString("@%1 : %2").arg( currentPost().author.userName ).arg(currentPost().content);
    //emit resendPost(txt);
    //QString txt = QString("||@%1 :\r\n").arg( currentPost().author.userName );
    emit reply( "", "2_"+currentPost().postId, currentPost().author.userName );//rt with comment

}

QString TencentPostWidget::generateSign()
{
    QString ss;
    TencentMicroBlog* microblog = dynamic_cast<TencentMicroBlog*>(currentAccount()->microblog());
    ss = "<b><a href='"+ microblog->profileUrl( currentAccount(),currentPost().author.userName )
         +"' title=\"" +
    currentPost().author.description + "\">" + currentPost().author.realName +
    "</a> - </b>";

    ss += "<a href=\"" + currentPost().link +
    "\" title=\"" + currentPost().creationDateTime.toString(Qt::DefaultLocaleLongDate) + "\">%1</a>";

    if( !currentPost().source.isNull() )
        ss += " - " + currentPost().source;

    return ss;
}


void TencentPostWidget::slotReply()
{
}

void TencentPostWidget::slotWrite()
{
    emit reply( QString("@%1").arg( currentPost().author.userName ), "3_"+currentPost().author.userName, currentPost().author.userName );//private msg
}

void TencentPostWidget::slotReplyAll()
{
    QString txt = QString("@%1 : %2").arg( currentPost().author.userName ).arg(currentPost().content);
    emit reply( txt, "1_"+currentPost().postId, currentPost().author.userName );//direct reply
}

void TencentPostWidget::slotFavorite()
{
    //setReadInternal();
    TencentMicroBlog* microblog = dynamic_cast<TencentMicroBlog*>(currentAccount()->microblog());
    Choqok::Post* post = new Choqok::Post;
    post->postId = currentPost().postId;
    if ( currentPost().isFavorited )
        microblog->removeFavorite( currentAccount(), post );
    else
        microblog->createFavorite( currentAccount(), post );
}

void TencentPostWidget::slotFavoriteCreated( Choqok::Account* account, Choqok::Post* post )
{
    if ( currentAccount() != account || post->postId != currentPost().postId )
        return;
    //delete post;
    Choqok::Post tmp = currentPost();
    tmp.isFavorited = true;
    setCurrentPost( tmp );
    favoriteButton->setChecked( true );
    favoriteButton->setIcon( KIcon( "rating" ) );
}

void TencentPostWidget::slotFavoriteRemoved( Choqok::Account* account, Choqok::Post* post )
{
    if ( currentAccount() != account || post->postId != currentPost().postId )
        return;

    //delete post;
    Choqok::Post tmp = currentPost();
    tmp.isFavorited = false;
    setCurrentPost( tmp );
    favoriteButton->setChecked( false );
    favoriteButton->setIcon(unFavIcon);
}
