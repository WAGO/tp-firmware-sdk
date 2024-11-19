/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef WEBVIEW_H
#define WEBVIEW_H

#include "alarmindication.h"
#include "msgtool.h"
#include <QIcon>
#include <QWebEngineView>

enum { CONN_FAILED = -1, CONN_SUCCESS, CONN_UNDEFINED};

typedef struct stSslList
{
  QList< QList<int> > confirmed_numbers; ///< saved SSL numbers already confirmed for same host
  QList<QString> confirmed_hosts;        ///< saved hosts with already confirmed SSL errors
  int browser_security_level;            ///< security level, f.e. low or high
  QList<int> low_enabled;                ///< security profile, allowed SSL errors in security level low
  QString errorText;                     ///< SSL error text
  QString cbText;                        ///< Checkbox-Text shown in confirm Message f.e. always allowed
}tSslInfo;


QT_BEGIN_NAMESPACE
class QAuthenticator;
class QMouseEvent;
class QNetworkProxy;
class QNetworkReply;
class QSslError;
QT_END_NAMESPACE

class BrowserMainWindow;
class WebPage : public QWebEnginePage
{
  Q_OBJECT
public:
  WebPage(QWebEngineProfile *profile, QObject *parent = 0);
  BrowserMainWindow *mainWindow();
  bool bCertificateError;
  QList<int> listScrollbarVertPositions;
  QList<int> listScrollbarHorizPositions;
  void ExecuteJsScrollbarVertCode();
  void ExecuteJsScrollbarHorizCode();
  QString ReadFile(QString sFile);  

protected:
  QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) Q_DECL_OVERRIDE;
#if !defined(QT_NO_UITOOLS)
  QObject *createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames,
                        const QStringList &paramValues);
#endif
  virtual bool certificateError(const QWebEngineCertificateError &error) Q_DECL_OVERRIDE;

public slots:
  void slotClearScrollbarLists();

private slots:
#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
  void handleUnsupportedContent(QNetworkReply *reply);
#endif
  void authenticationRequired(const QUrl &requestUrl, QAuthenticator *auth);
  void proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost);
  //void loadFinished(bool ok);
  void onPageWbm();

private:
  friend class WebView;

  // set the webview mousepressedevent
  Qt::KeyboardModifiers m_keyboardModifiers;
  Qt::MouseButtons m_pressedButtons;

  int showSslError(QString sTxt, bool bShowCheckbox = false);
  tSslInfo sslInfo;

  bool isSslErrorConfirmed(QString host, int nErr);
  bool setSslErrorConfirmed(QString host, int nErr);

  void ReadBrowserSecurityLevel();
  void ReadSslAllowedFromFile();
  int WriteSslAllowedToFile(QString host, int nErr);
  QString sJsGetScrollbarVertCode;
  QString sJsGetScrollbarHorizCode;
  
//protected:
  //bool event(QEvent *e);
};

class WebView : public QWebEngineView
{
  Q_OBJECT

public:
  WebView(QWidget *parent = 0);
  ~WebView();
  WebPage *webPage() const
  {
    return m_page;
  }
  void setPage(WebPage *page);

  void loadUrl(const QUrl &url);
  QUrl url() const;

  inline int progress() const
  {
    return m_progress;
  }

  AlarmIndication reconnectPage;
  QTimer * m_pTimerCommCheck;
  QTimer * m_pTimerReconnect;
  QString jsPDF;
  QUrl currentUrl;

  void SetTcpRxTimestamp(QString sIp, int iTimestamp);
  void RemoveMonitorTmpFile();
  void CreateMonitorTmpFile();
  bool IsCurrentWebView();
  bool IsMonitoringConfigured();
  bool IsLocalhost(QUrl u);
  
  void qLog(QString s);

protected:
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void contextMenuEvent(QContextMenuEvent *event);
  void wheelEvent(QWheelEvent *event);

  //bool event(QEvent *e);

private slots:
  void setProgress(int progress);
  void loadFinished(bool success);
  void loadStarted();
  void onFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature);
  void onIconChanged(const QIcon &icon);
  void onReconnect();
  void onShowReconnectPage();
  void onCheckError500InternalServerError();
  void onCheckVisuConnection();

public slots:
  void onShowWbm(QString sWbm);  

private:
  QUrl m_initialUrl;
  int m_progress;
  WebPage *m_page;
  int GetConnectionState();
  int m_iTcpRxTimestamp;
  bool IsLocalhost();
  QString GetLocalIpAddress(QString sAdapter);
};

#endif
