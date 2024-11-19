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

#include "browserapplication.h"
#include "browsermainwindow.h"
#include "cookiejar.h"
#include "downloadmanager.h"
#include "featurepermissionbar.h"
#include "ui_passworddialog.h"
#include "ui_proxy.h"
#include "tabwidget.h"
#include "webview.h"

#include <QtGui/QClipboard>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtGui/QMouseEvent>

#include <QWebEngineContextMenuData>
#include <QWebEngineSettings>

#ifndef QT_NO_UITOOLS
#include <QtUiTools/QUiLoader>
#endif  //QT_NO_UITOOLS

#include <QtCore/QDebug>
#include <QtCore/QBuffer>
#include <QtCore/QTimer>
#include <QNetworkInterface>
#include <QLocale>
#include <QLatin1String>

#include <QFile>
#include <QDateTime>

//#include <QGestureEvent>
#include <time.h>
#include "globals.h"

#include <QWebEngineCertificateError>

#define SSL_CB_OFFSET         100
#define SSL_CB_TEXT           "Always allowed"


WebPage::WebPage(QWebEngineProfile *profile, QObject *parent)
  : QWebEnginePage(profile, parent)
  , m_keyboardModifiers(Qt::NoModifier)
  , m_pressedButtons(Qt::NoButton)
{
 
#if defined(QWEBENGINEPAGE_SETNETWORKACCESSMANAGER)
  setNetworkAccessManager(BrowserApplication::networkAccessManager());
#endif
#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
  connect(this, SIGNAL(unsupportedContent(QNetworkReply*)),
          this, SLOT(handleUnsupportedContent(QNetworkReply*)));
#endif
  connect(this, SIGNAL(authenticationRequired(const QUrl &, QAuthenticator*)),
          SLOT(authenticationRequired(const QUrl &, QAuthenticator*)));
  connect(this, SIGNAL(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)),
          SLOT(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)));

  //connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

  sslInfo.browser_security_level = BROWSER_SEC_HIGH;
  bCertificateError = false;

  ReadBrowserSecurityLevel();
  ReadSslAllowedFromFile();
  sJsGetScrollbarVertCode = ReadFile("/etc/specific/webengine/getscrollbarpos_v.js");
  sJsGetScrollbarHorizCode = ReadFile("/etc/specific/webengine/getscrollbarpos_h.js");

}

BrowserMainWindow *WebPage::mainWindow()
{
  QObject *w = this->parent();
  while (w)
  {
    if (BrowserMainWindow *mw = qobject_cast<BrowserMainWindow*>(w))
      return mw;
    w = w->parent();
  }
  return BrowserApplication::instance()->mainWindow();
}


/// \brief show SSL Error message box
/// \param[in]  sTxt      message text
/// \param[in]  bAllowed  load url is allowed or not
///
int WebPage::showSslError(QString sTxt, bool bShowCheckbox)
{
  char szCmd[512] = "";
  QByteArray ba;
  ba.append(sTxt); // = sTxt.toLatin1();
  int sysret;

  //avoid gray rectangle
  //webview->repaint();

  WebView * pView = qobject_cast<WebView*>(this->view());
  if (pView)
  {
    pView->reconnectPage.Hide();
    pView->reconnectPage.update();
    pView->update();
  }

  if (bShowCheckbox)
  {
    if (sslInfo.cbText.isEmpty())
    {
      sslInfo.cbText = SSL_CB_TEXT;
    }
    QByteArray cb = sslInfo.cbText.toLatin1();

    sprintf(szCmd, "dialogbox \"%s\" %s %s -c \"%s\" -d 2 -s 30", ba.data(), "CONTINUE", "CANCEL", cb.data() );
    sysret = system(szCmd);
    sysret = WEXITSTATUS(sysret);

    if ((sysret == 255) || (sysret < 0))
    {
      sprintf(szCmd, "dialogbox \"%s\" %s %s -d 2 -s 10", ba.data(), "CONTINUE", "CANCEL");
      sysret = system(szCmd);
      sysret = WEXITSTATUS(sysret);
    }
  }
  else
  {
    sprintf(szCmd, "dialogbox \"%s\" %s -s 10", ba.data(), "OK" );
    sysret = system(szCmd);
    sysret = WEXITSTATUS(sysret);
  }

  //qDebug() << "showSslError: " << sTxt << " " << sysret;

  return sysret;
}


/*
QSslError::NoError	0

NOT ALLOWED
QSslError::UnableToGetIssuerCertificate	1
QSslError::UnableToDecryptCertificateSignature	2
QSslError::UnableToDecodeIssuerPublicKey	3
QSslError::CertificateSignatureFailed	4

QSslError::InvalidNotBeforeField	7
QSslError::InvalidNotAfterField	8

QSslError::UnableToGetLocalIssuerCertificate	11
QSslError::UnableToVerifyFirstCertificate	12
QSslError::CertificateRevoked	13
QSslError::InvalidCaCertificate	14
QSslError::PathLengthExceeded	15
QSslError::InvalidPurpose	16
QSslError::CertificateUntrusted	17
QSslError::CertificateRejected	18
QSslError::SubjectIssuerMismatch	19
QSslError::AuthorityIssuerSerialNumberMismatch	20
QSslError::NoPeerCertificate	21
QSslError::HostNameMismatch	22
QSslError::UnspecifiedError	-1
QSslError::NoSslSupport	23
QSslError::CertificateBlacklisted	24


ALLOWED
QSslError::CertificateNotYetValid	5
QSslError::CertificateExpired	6
QSslError::SelfSignedCertificate	9
QSslError::SelfSignedCertificateInChain	10

*/




bool WebPage::isSslErrorConfirmed(QString host, int nErr)
{
  bool bRet = false;
  int index = sslInfo.confirmed_hosts.indexOf(host);
  if (index >= 0)
  {
    QList<int> list = sslInfo.confirmed_numbers.at(index);
    if (list.contains(nErr))
    {
      bRet = true;
    }
//    else
//    {
//      qDebug() << "NUMBER NOT FOUND " << nErr;
//    }

//    foreach (int k, list)
//    {
//      qDebug() << "NUMBER int: " << k;
//    }

  }

//  else
//  {
//    qDebug() << "HOST NOT FOUND";
//  }

  //qDebug() << "isSslErrorConfirmed: " << host << " error: " << nErr << " ret: " << bRet;

  return bRet;
}

bool WebPage::setSslErrorConfirmed(QString host, int nErr)
{
  bool bRet = false;
  QList<int> list;

  //qDebug() << "setSslErrorConfirmed" << host << " " << nErr;

  if (sslInfo.confirmed_hosts.contains(host) == false)
  {
    list.append(nErr);
    sslInfo.confirmed_hosts.append(host);
    sslInfo.confirmed_numbers.append(list);
    bRet = true;
  }
  else
  {
    int index = sslInfo.confirmed_hosts.indexOf(host);
    if (index >= 0)
    {
      list = sslInfo.confirmed_numbers.at(index);
      if (list.contains(nErr)==false)
      {
        list.append(nErr);
        sslInfo.confirmed_numbers[index] = list;
        bRet = true;
      }
    }
  }

/*
  foreach (int k, list)
  {
    qDebug() << "debug int: " << k;
  }
  foreach (QString s, sslInfo.hosts)
  {
    qDebug() << "debug string: " << s;
  }
  qDebug() << "setSslErrorConfirmed: " << host << " error: " << nErr << " ret: " << bRet;
*/

  return bRet;
}

QString WebPage::ReadFile(QString sFile)
{
  QString sTxt;
  QFile file(sFile);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
     QTextStream in(&file);
     sTxt = in.readAll();
     file.close();
  }
  return sTxt;
}

void WebPage::ReadBrowserSecurityLevel()
{
  //qDebug() << "ReadBrowserSecurityLevel";

  tConfList * pList = NULL;
  char szOut[256] = "";
  int ret;

  //create list
  pList = ConfCreateList();
  if (pList)
  {
    ret = ConfReadValues(pList, (char*) CONF_FILE_SECURITY);
    if (ret != SUCCESS)
    {
      ConfDestroyList(pList);
      return;
    }

    if (ConfGetValue(pList, (char*)"level", szOut, sizeof(szOut)) == SUCCESS)
    {
       if (stricmp(szOut, "low") == 0)
       {
         //qDebug() << "BROWSER_SEC_LOW";
         sslInfo.browser_security_level = BROWSER_SEC_LOW;
       }
    }

    if (ConfGetValue(pList, (char*)"low_enabled", szOut, sizeof(szOut)) == SUCCESS)
    {
      QString var(szOut);
      QStringList sl = var.split(",");
      sslInfo.low_enabled.clear();
      foreach (const QString &s, sl)
      {
        //qDebug() << s;
        sslInfo.low_enabled.append(s.toInt());
      }
    }

    //if (ConfGetValue(pList, "check_datetime", szOut, sizeof(szOut)) == SUCCESS)
    //{
    //  if (stricmp(szOut, "no") == 0)
    //  {
    //    bCheckDateTime = false;
    //  }
    //}

    if (ConfGetValue(pList, (char*)"checkbox_text", szOut, sizeof(szOut)) == SUCCESS)
    {
      QString var(szOut);
      sslInfo.cbText = var;
    }

    ConfDestroyList(pList);
  }

}


void WebPage::ReadSslAllowedFromFile()
{
  //qDebug() << "ReadSslAllowedFromFile";
  QFile inputFile(CONF_FILE_SSLALLOWED);
  if (inputFile.open(QIODevice::ReadOnly))
  {
     QTextStream in(&inputFile);
     while (!in.atEnd())
     {
        QString line = in.readLine();
        QStringList sl = line.split("=");
        if (sl.size() == 2)
        {
          QString sHost = sl.at(0);
          QString sSslCodes = sl.at(1);
          sHost = sHost.trimmed();
          sSslCodes = sSslCodes.trimmed();

          QStringList slCodes = sSslCodes.split(",");
          foreach (const QString &s, slCodes)
          {
            //qDebug() << s;
            QString sCode = s.trimmed();
            //qDebug() << sHost << " " << sCode;
            setSslErrorConfirmed(sHost, sCode.toInt());
          }
        }
     }
     inputFile.close();
  }
}

int WebPage::WriteSslAllowedToFile(QString host, int nErr)
{
  int retval = -1;
  QFileInfo fi(CONF_FILE_SSLALLOWED);
  QFile inputFile(CONF_FILE_SSLALLOWED);
  if (fi.isFile() && fi.exists())
  {
    //file exists, read complete file content into string list
    QStringList slFile;
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
         QString s = in.readLine();
         slFile.append(s);
       }
       inputFile.close();
    }


    bool bFound = false;
    int i=0;
    foreach (const QString &sLine, slFile)
    {
      QStringList sl = sLine.split("=");
      if (sl.size() == 2)
      {
        QString sHost = sl.at(0);
        QString sSslCodes = sl.at(1);
        sHost = sHost.trimmed();
        sSslCodes = sSslCodes.trimmed();

        if (sHost.compare(host, Qt::CaseInsensitive) == 0)
        {
          QStringList slCodes = sSslCodes.split(",");
          bFound = true;
          if (slCodes.contains(QString::number(nErr)) == false)
          {
            //append new ssl code
            QString sNew = sLine + "," + QString::number(nErr);
            slFile.replace(i, sNew);
          }
        }
      }
      i++;
    }

    if (bFound == false)
    {
      //append new line
      QString sNewLine = host + "=" + QString::number(nErr);
      slFile.append(sNewLine);
    }

    //write complete file content into string list
    if (inputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
       QTextStream out(&inputFile);
       foreach (const QString &sLine, slFile)
       {
         out << sLine << endl;
       }
       out.flush();
       inputFile.close();
       retval = 0;
    }

  }
  else
  {
    //file not exists, create one
    if (inputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
      QTextStream out(&inputFile);
      QString s = host;
      s += "=";
      s += QString::number(nErr);

      out << s  << endl;
      out.flush();
      inputFile.close();
      retval = 0;
    }
  }

  return retval;
}


/*

QWebEngineCertificateError::SslPinnedKeyNotInCertificateChain	-150	The certificate did not match the built-in public key pins for the host name.
QWebEngineCertificateError::CertificateCommonNameInvalid	-200      The certificate's common name did not match the host name.
QWebEngineCertificateError::CertificateDateInvalid	-201            The certificate is not valid at the current date and time.
QWebEngineCertificateError::CertificateAuthorityInvalid	-202        The certificate is not signed by a trusted authority.
QWebEngineCertificateError::CertificateContainsErrors	-203          The certificate contains errors.
QWebEngineCertificateError::CertificateNoRevocationMechanism	-204	The certificate has no mechanism for determining if it has been revoked.
QWebEngineCertificateError::CertificateUnableToCheckRevocation	-205	Revocation information for the certificate is not available.
QWebEngineCertificateError::CertificateRevoked	-206                The certificate has been revoked.
QWebEngineCertificateError::CertificateInvalid	-207                The certificate is invalid.
QWebEngineCertificateError::CertificateWeakSignatureAlgorithm	-208	The certificate is signed using a weak signature algorithm.
QWebEngineCertificateError::CertificateNonUniqueName	-210          The host name specified in the certificate is not unique.
QWebEngineCertificateError::CertificateWeakKey	-211                The certificate contains a weak key.
QWebEngineCertificateError::CertificateNameConstraintViolation	-212	The certificate claimed DNS names that are in violation of name constraints.  

*/


bool WebPage::certificateError(const QWebEngineCertificateError &error)
{
  int exitCode;
  int errorNumber = error.error();
  QList<int> numbers;
  bool bIgnoreError = false;
  sslInfo.errorText = tr("If you wish so, you may continue with an unverified certificate. "
                        "Accepting an unverified certificate means "
                        "you may not be connected with the host you tried to connect to.\n"
                        "Do you wish to override the security check and continue?");


  qDebug() << "Certificate Error: " << errorNumber;
  qDebug() << error.errorDescription();
  qDebug() << error.isOverridable();

  //WBM - local webserver using https self signed certificate ...
  WebView * pView = qobject_cast<WebView*>(this->view());
  if (pView)
  {
    if (pView->IsLocalhost(error.url()))
    {
       bIgnoreError = true;
       return bIgnoreError;
    }
  }

  if ((sslInfo.browser_security_level == BROWSER_SEC_LOW) && (sslInfo.low_enabled.contains(errorNumber)==true))
  {
    //low_enabled in /etc/specific/browsersecurity
    qDebug() << "This error is in low_enabled list";
    bIgnoreError = true;
  }
  else if (sslInfo.browser_security_level == BROWSER_SEC_HIGH)
  {
    QString host = error.url().host().toLower();
    sslInfo.errorText = host + "\n" + error.errorDescription() + "\n" + "Do you wish to override the security check and continue?";
    if (isSslErrorConfirmed(host, errorNumber)==false)
    {
      exitCode = showSslError(sslInfo.errorText, true);
      if (exitCode == 1)
      {
        bIgnoreError = true;
      }
      else if (exitCode == 1+SSL_CB_OFFSET)
      {
        bIgnoreError = true;
        setSslErrorConfirmed(host, errorNumber);
        WriteSslAllowedToFile(host, errorNumber);
      }
      else
      {
        bIgnoreError = false;
      }

    }
    else
    {
      bIgnoreError = true;
      qDebug() << "This error is in saved file list";
    }
  }
  else if (error.isOverridable())
  {
    //"The certificate is not yet valid.";
    exitCode = showSslError(sslInfo.errorText, false);
    if (exitCode == 1)
    {
      bIgnoreError = true;
    }
    else
    {
      bIgnoreError = false;
    }
  }
  else
  {
    //not overrideable, show only an OK Button and goto WBM
    sslInfo.errorText = error.errorDescription();
    showSslError(sslInfo.errorText, false);
  }

  //show WBM if error not ignored
  if (bIgnoreError == false)
  {
    bCertificateError = true;
    QTimer::singleShot(1000, this, SLOT(onPageWbm()));
  }

  return  bIgnoreError;
}

void WebPage::onPageWbm()
{
  WebView * pView = qobject_cast<WebView*>(this->view());
  if (pView)
  {
    QString sUrl = "https://127.0.0.1/wbm/index.html";
    if (g_webengine.slUrls.count()>0)
    {
      sUrl = g_webengine.slUrls.at(0);
    }
    pView->onShowWbm(sUrl);
  }
  else
  {  
    qDebug() << "certificateError: LoadWBM";
    QUrl UrlWbm("https://127.0.0.1/wbm/index.html");
    load(UrlWbm);
  }
}

class PopupWindow : public QWidget
{
  Q_OBJECT
public:
  PopupWindow(QWebEngineProfile *profile)
    : m_addressBar(new QLineEdit(this))
    , m_view(new WebView(this))
  {
    m_view->setPage(new WebPage(profile, m_view));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);
    layout->addWidget(m_addressBar);
    layout->addWidget(m_view);
    m_view->setFocus();

    connect(m_view, &WebView::titleChanged, this, &QWidget::setWindowTitle);
    connect(m_view, &WebView::urlChanged, this, &PopupWindow::setUrl);
    connect(page(), &WebPage::geometryChangeRequested, this, &PopupWindow::adjustGeometry);
    connect(page(), &WebPage::windowCloseRequested, this, &QWidget::close);
  }

  QWebEnginePage* page() const
  {
    return m_view->page();
  }

private Q_SLOTS:
  void setUrl(const QUrl &url)
  {
    m_addressBar->setText(url.toString());
  }

  void adjustGeometry(const QRect &newGeometry)
  {
    const int x1 = frameGeometry().left() - geometry().left();
    const int y1 = frameGeometry().top() - geometry().top();
    const int x2 = frameGeometry().right() - geometry().right();
    const int y2 = frameGeometry().bottom() - geometry().bottom();

    setGeometry(newGeometry.adjusted(x1, y1 - m_addressBar->height(), x2, y2));
  }

private:
  QLineEdit *m_addressBar;
  WebView *m_view;

};

#include "webview.moc"

QWebEnginePage *WebPage::createWindow(QWebEnginePage::WebWindowType type)
{
  if (type == QWebEnginePage::WebBrowserTab)
  {
    return mainWindow()->tabWidget()->newTab()->page();
  }
  else if (type == QWebEnginePage::WebBrowserBackgroundTab)
  {
    return mainWindow()->tabWidget()->newTab(false)->page();
  }
  else if (type == QWebEnginePage::WebBrowserWindow)
  {
    BrowserApplication::instance()->newMainWindow();
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();
    return mainWindow->currentTab()->page();
  }
  else
  {
    PopupWindow *popup = new PopupWindow(profile());
    popup->setAttribute(Qt::WA_DeleteOnClose);
    popup->show();
    return popup->page();
  }
}

#if !defined(QT_NO_UITOOLS)
QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames,
                               const QStringList &paramValues)
{
  Q_UNUSED(url);
  Q_UNUSED(paramNames);
  Q_UNUSED(paramValues);
  QUiLoader loader;
  return loader.createWidget(classId, view());
}
#endif // !defined(QT_NO_UITOOLS)

#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
  QString errorString = reply->errorString();

  if (m_loadingUrl != reply->url())
  {
    // sub resource of this page
    qWarning() << "Resource" << reply->url().toEncoded() << "has unknown Content-Type, will be ignored.";
    reply->deleteLater();
    return;
  }

  if (reply->error() == QNetworkReply::NoError && !reply->header(QNetworkRequest::ContentTypeHeader).isValid())
  {
    errorString = "Unknown Content-Type";
  }

  QFile file(QLatin1String(":/notfound.html"));
  bool isOpened = file.open(QIODevice::ReadOnly);
  Q_ASSERT(isOpened);
  Q_UNUSED(isOpened)

  QString title = tr("Error loading page: %1").arg(reply->url().toString());
  QString html = QString(QLatin1String(file.readAll()))
                 .arg(title)
                 .arg(errorString)
                 .arg(reply->url().toString());

  QBuffer imageBuffer;
  imageBuffer.open(QBuffer::ReadWrite);
  QIcon icon = view()->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, view());
  QPixmap pixmap = icon.pixmap(QSize(32,32));
  if (pixmap.save(&imageBuffer, "PNG"))
  {
    html.replace(QLatin1String("IMAGE_BINARY_DATA_HERE"),
                 QString(QLatin1String(imageBuffer.buffer().toBase64())));
  }

  QList<QWebEngineFrame*> frames;
  frames.append(mainFrame());
  while (!frames.isEmpty())
  {
    QWebEngineFrame *frame = frames.takeFirst();
    if (frame->url() == reply->url())
    {
      frame->setHtml(html, reply->url());
      return;
    }
    QList<QWebEngineFrame *> children = frame->childFrames();
    foreach (QWebEngineFrame *frame, children)
      frames.append(frame);
  }
  if (m_loadingUrl == reply->url())
  {
    mainFrame()->setHtml(html, reply->url());
  }
}
#endif

void WebPage::authenticationRequired(const QUrl &requestUrl, QAuthenticator *auth)
{
  BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

  QDialog dialog(mainWindow);
  dialog.setWindowFlags(Qt::Sheet);

  Ui::PasswordDialog passwordDialog;
  passwordDialog.setupUi(&dialog);

  passwordDialog.iconLabel->setText(QString());
  passwordDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0,
                                      mainWindow).pixmap(32, 32));

  QString introMessage = tr("<qt>Enter username and password for \"%1\" at %2</qt>");
  introMessage = introMessage.arg(auth->realm()).arg(requestUrl.toString().toHtmlEscaped());
  passwordDialog.introLabel->setText(introMessage);
  passwordDialog.introLabel->setWordWrap(true);

  if (dialog.exec() == QDialog::Accepted)
  {
    auth->setUser(passwordDialog.userNameLineEdit->text());
    auth->setPassword(passwordDialog.passwordLineEdit->text());
  }
  else
  {
    // Set authenticator null if dialog is cancelled
    *auth = QAuthenticator();
  }
}

void WebPage::proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost)
{
  Q_UNUSED(requestUrl);
  BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

  QDialog dialog(mainWindow);
  dialog.setWindowFlags(Qt::Sheet);

  Ui::ProxyDialog proxyDialog;
  proxyDialog.setupUi(&dialog);

  proxyDialog.iconLabel->setText(QString());
  proxyDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0,
                                   mainWindow).pixmap(32, 32));

  QString introMessage = tr("<qt>Connect to proxy \"%1\" using:</qt>");
  introMessage = introMessage.arg(proxyHost.toHtmlEscaped());
  proxyDialog.introLabel->setText(introMessage);
  proxyDialog.introLabel->setWordWrap(true);

  if (dialog.exec() == QDialog::Accepted)
  {
    auth->setUser(proxyDialog.userNameLineEdit->text());
    auth->setPassword(proxyDialog.passwordLineEdit->text());
  }
  else
  {
    // Set authenticator null if dialog is cancelled
    *auth = QAuthenticator();
  }
}

void WebPage::ExecuteJsScrollbarVertCode()
{
  static QString jsResult = "";

  if (sJsGetScrollbarVertCode.isEmpty())
    return;

  //qDebug() << "ExecuteJsScrollbarVertCode: " << jsResult;

  //js code is asynchronous
  QStringList slScrollbarPositions = jsResult.split(";", QString::SkipEmptyParts);
  foreach (const QString &s, slScrollbarPositions)
  {
    int iNewValue = s.toInt();
    if (iNewValue > 0)
    {
      if (!listScrollbarVertPositions.contains(iNewValue))
      {
        listScrollbarVertPositions.append(iNewValue);
      }
    }
  }

  this->runJavaScript(sJsGetScrollbarVertCode, [](const QVariant &v)
  {
    //js code / callback is asynchronous
    jsResult = v.toString();
  });

}

void WebPage::ExecuteJsScrollbarHorizCode()
{
  static QString jsResult = "";

  if (sJsGetScrollbarHorizCode.isEmpty())
    return;

  //qDebug() << "ExecuteJsScrollbarHorizCode: " << jsResult;

  //js code is asynchronous
  QStringList slScrollbarPositions = jsResult.split(";", QString::SkipEmptyParts);
  foreach (const QString &s, slScrollbarPositions)
  {
    int iNewValue = s.toInt();
    if (iNewValue > 0)
    {
      if (!listScrollbarHorizPositions.contains(iNewValue))
      {
        listScrollbarHorizPositions.append(iNewValue);
      }
    }
  }

  this->runJavaScript(sJsGetScrollbarHorizCode, [](const QVariant &v)
  {
    //js code / callback is asynchronous
    jsResult = v.toString();
  });

}

void WebPage::slotClearScrollbarLists()
{
  //qDebug() << "slotClearScrollbarLists";
  listScrollbarVertPositions.clear();
  listScrollbarHorizPositions.clear();
}

WebView::~WebView()
{
  if (m_pTimerCommCheck)
  {
    delete m_pTimerCommCheck;
    m_pTimerCommCheck = NULL;
  }

  if (m_pTimerReconnect)
  {
    delete m_pTimerReconnect;
    m_pTimerReconnect = NULL;
  }
}

WebView::WebView(QWidget* parent)
  : QWebEngineView(parent)
  , m_progress(0)
  , m_page(0)
{  
  setWindowFlags(Qt::FramelessWindowHint);

  QRect mainScreenSize = QApplication::desktop()->screenGeometry();
  setGeometry(mainScreenSize);
  m_iTcpRxTimestamp = 0;
  
  //setAttribute(Qt::WA_AcceptTouchEvents, true);
  
  //QLocale l(QLocale::Germany, QLocale::Egypt);
  //QLocale loc("en_US");
  //setLocale(loc);

  // disable context menu
  QWebEngineView::setContextMenuPolicy(Qt::CustomContextMenu);

  QFile file;
  file.setFileName("/etc/specific/webengine/pdf/ePDF.js");
  file.open(QIODevice::ReadOnly);
  jsPDF = file.readAll();
  file.close();

  m_pTimerReconnect = new QTimer(this);
  if (m_pTimerReconnect)
  {
    m_pTimerReconnect->setInterval(1000);
    connect(m_pTimerReconnect, SIGNAL(timeout()), this, SLOT(onReconnect()));
  }

  m_pTimerCommCheck = new QTimer(this);
  if (m_pTimerCommCheck)
  {
    m_pTimerCommCheck->setInterval(g_webengine.iReconnectInterval * 1000);
    connect(m_pTimerCommCheck, SIGNAL(timeout()), this, SLOT(onCheckVisuConnection()));
  }
    
  connect(&reconnectPage, SIGNAL(signalStartWbm(QString)), this, SLOT(onShowWbm(QString)));
  
  connect(this, SIGNAL(loadProgress(int)), this, SLOT(setProgress(int)));
  connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
  connect(this, SIGNAL(loadStarted()), this, SLOT(loadStarted()));

  connect(this, &QWebEngineView::renderProcessTerminated,
          [=](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode)
  {
    const char *status = "";
    switch (termStatus)
    {
    case QWebEnginePage::NormalTerminationStatus:
      status = "(normal exit)";
      break;
    case QWebEnginePage::AbnormalTerminationStatus:
      status = "(abnormal exit)";
      break;
    case QWebEnginePage::CrashedTerminationStatus:
      status = "(crashed)";
      break;
    case QWebEnginePage::KilledTerminationStatus:
      status = "(killed)";
      break;
    }

    qInfo() << "Render process exited with code" << statusCode << status;
    QTimer::singleShot(0, [this] { reload(); });
  });
    
  //BrowserApplication::instance()->setPrivateBrowsing(false);
    
  //qDebug() << "WebView width: " << width() << " height: " << height();

  if (g_webengine.slUrls.count() > 0)
    reconnectPage.SetWbmUrl(g_webengine.slUrls.at(0));
}

void WebView::setPage(WebPage *_page)
{
  m_page = _page;
  QWebEngineView::setPage(_page);

  disconnect(page(), &QWebEnginePage::iconChanged, this, &WebView::iconChanged);
  connect(page(), SIGNAL(iconChanged(QIcon)),
          this, SLOT(onIconChanged(QIcon)));
  connect(page(), &WebPage::featurePermissionRequested, this, &WebView::onFeaturePermissionRequested);
#if defined(QWEBENGINEPAGE_UNSUPPORTEDCONTENT)
  page()->setForwardUnsupportedContent(true);
#endif
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu;
    if (page()->contextMenuData().linkUrl().isValid()) {
        menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose, true);
        menu->addAction(page()->action(QWebEnginePage::OpenLinkInThisWindow));
        menu->addAction(page()->action(QWebEnginePage::OpenLinkInNewWindow));
        menu->addAction(page()->action(QWebEnginePage::OpenLinkInNewTab));
        menu->addAction(page()->action(QWebEnginePage::OpenLinkInNewBackgroundTab));
        menu->addSeparator();
        menu->addAction(page()->action(QWebEnginePage::DownloadLinkToDisk));
        menu->addAction(page()->action(QWebEnginePage::CopyLinkToClipboard));
    } else {
        menu = page()->createStandardContextMenu();
    }
    if (page()->contextMenuData().selectedText().isEmpty())
        menu->addAction(page()->action(QWebEnginePage::SavePage));
    menu->popup(event->globalPos());
}

void WebView::wheelEvent(QWheelEvent *event)
{
/*
  qDebug() << "-----------------------------";
  qDebug() << "wheelEvent pos: " << event->pos();
  qDebug() << "wheelEvent globalPosF: " << event->globalPosF();
  qDebug() << "wheelEvent pixelDelta: " << event->pixelDelta();
  qDebug() << "wheelEvent angleDelta: " << event->angleDelta();
  qDebug() << "wheelEvent delta: " << event->delta();
  qDebug() << "wheelEvent orientation: " << event->orientation();
  qDebug() << "wheelEvent buttons: " << event->buttons();
  qDebug() << "wheelEvent modifiers: " << event->modifiers();  
  qDebug() << "wheelEvent isAccepted: " << event->isAccepted();
*/
  
#if defined(_________QWEBENGINEPAGE_SETTEXTSIZEMULTIPLIER)
  if (QApplication::keyboardModifiers() & Qt::ControlModifier)
  {
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
    event->accept();
    return;
  }
#endif
    
  QWebEngineView::wheelEvent(event);
}

void WebView::onFeaturePermissionRequested(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
{
  FeaturePermissionBar *permissionBar = new FeaturePermissionBar(this);
  connect(permissionBar, &FeaturePermissionBar::featurePermissionProvided, page(), &QWebEnginePage::setFeaturePermission);

  // Discard the bar on new loads (if we navigate away or reload).
  connect(page(), &QWebEnginePage::loadStarted, permissionBar, &QObject::deleteLater);

  permissionBar->requestPermission(securityOrigin, feature);
}

void WebView::setProgress(int progress)
{
  m_progress = progress;
  //qDebug() << "progress: " << m_progress;  
  if (progress==100)
  {
    WebPage * pPage = qobject_cast<WebPage*>(page());
    pPage->runJavaScript(jsPDF);
    pPage->runJavaScript("qt.replacePDFLinks(); undefined");
  }
}

void WebView::loadStarted()
{
  if (m_pTimerCommCheck)
  {
    m_pTimerCommCheck->stop();
  }  

  if ((g_webengine.bReconnect) && (reconnectPage.isVisible()))
  {
    //try to connect message
    QString s = g_webengine.sConnectedText;
    reconnectPage.SetLabelText(s.replace("%U", url().toString()));
  }

  WebPage * pPage = qobject_cast<WebPage*>(page());
  if (pPage)
    pPage->slotClearScrollbarLists();
}

void WebView::qLog(QString s)
{
    QFile data("/tmp/webview.txt");
    QDateTime dteNow = QDateTime::currentDateTime();
    QString sNow = dteNow.toString("hh:mm:ss");
    if (data.open(QFile::WriteOnly | QFile::Append)) {
        QTextStream out(&data);
        out << sNow << ": " << s << "\n";
        data.close();
    }
}

void WebView::loadFinished(bool success)
{
  //qDebug() << "loadFinished success: " << success << " url: " << url();
  if (success)
  {
    if (100 != m_progress)
    {
      qWarning() << "Received finished signal while progress is still:" << progress() << "Url:" << url();
    }
    else
    {
      //load success and progress 100

      if ((g_webengine.bReconnect)&&(m_pTimerCommCheck))
      {
        m_pTimerCommCheck->stop();
      }

      if (IsCurrentWebView())
      {
        m_iTcpRxTimestamp = 0;
        if (g_webengine.bReconnect)
        {
          //do a continuous connection check every 10 s
          if (m_pTimerCommCheck)
          {
            //write tmp file used by tcp_monitor
            CreateMonitorTmpFile();
            m_pTimerCommCheck->start();
          }
        }

      }
    }
  }

  WebPage * pPage = qobject_cast<WebPage*>(page());

  if (pPage)
  {

    if (pPage->bCertificateError == true)
    {
        //do not reconnect in case of certificate error
        pPage->bCertificateError = false;
    }
    else if (g_webengine.bReconnect)
    {

      //todo Alarmhandling
      if ((success == false) && (m_progress == 0))
      {
        //show reconnect page with countdown
        QTimer::singleShot(200, this, SLOT(onShowReconnectPage()));
      }
      else if ((success == false) && (m_progress == 100))
      {
          //QT internal dialog ( Website ist nicht erreichbar ) ERR_ADDRESS_UNREACHABLE
          //show reconnect page with countdown
          if (IsLocalhost(url()) == false)
          {
            QTimer::singleShot(200, this, SLOT(onShowReconnectPage()));
          }
          return;
      }
      else
      {
        reconnectPage.Hide();
        QTimer::singleShot(200, this, SLOT(onCheckError500InternalServerError()));
      }

    }

    if (success)
    {
      pPage->runJavaScript("document.getElementsByName('viewport').length;",[=](QVariant result){
          int nViewports = result.toInt();
          //qDebug() << "Viewports: " << nViewports;
          if (nViewports>0)
          {
              pPage->runJavaScript("document.getElementsByName('viewport')[0].content;", [=](QVariant resContent){
                  QString strContent = resContent.toString();
                  strContent = strContent.trimmed();
                  //qLog(strContent);
                  if (strContent.length() > 3)
                  {
                    bool bRun = false;
                    //qDebug() << "Viewport content: " << strContent;
                    if (strContent.contains("minimum-scale", Qt::CaseInsensitive)==false)
                    {
                        strContent = strContent+", minimum-scale=1.0";
                        bRun = true;
                    }
                    if(strContent.contains("user-scalable=no") && (strContent.contains("initial-scale")==false))
                    {
                        strContent = strContent + ", initial-scale=1.0";
                        bRun = true;
                    }
                    if (bRun)
                    {
                        //qDebug() << "going to set:" << "document.getElementsByName('viewport')[0].content=\""+strContent+"\"";
                        pPage->runJavaScript("document.getElementsByName('viewport')[0].content=\""+strContent+"\"", [=](QVariant res){
                            //qDebug() << "result:" << res.toString();
                        });
                    }
                  }
              });
          }
      });
    }
  }

  m_progress = 0;
}

const QString cPdfViewerUrl("file:////etc/specific/webengine/pdf/web/viewer.html");
void WebView::loadUrl(const QUrl &url)
{
  m_initialUrl = url;
  currentUrl = url;
  if (url.fileName().right(4).compare(".pdf", Qt::CaseInsensitive)==0)
  {
    QString strURL = cPdfViewerUrl;

    strURL.append("?file=");
    strURL.append(url.toString());
    QUrl url2(strURL);

    load(url2);
  }
  else
  {
    load(url);
  }
}


QUrl WebView::url() const
{
  QUrl url = QWebEngineView::url();
  if (!url.isEmpty())
    return url;

  return m_initialUrl;
}

void WebView::onIconChanged(const QIcon &icon)
{
  if (icon.isNull())
    emit iconChanged(BrowserApplication::instance()->defaultIcon());
  else
    emit iconChanged(icon);
}

void WebView::mousePressEvent(QMouseEvent *event)
{
  m_page->m_pressedButtons = event->buttons();
  m_page->m_keyboardModifiers = event->modifiers();
  QWebEngineView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent *event)
{
  QWebEngineView::mouseReleaseEvent(event);
  if (!event->isAccepted() && (m_page->m_pressedButtons & Qt::MidButton))
  {
    QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
    if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty())
    {
      setUrl(url);
    }
  }
}

void WebView::onShowWbm(QString sWbm)
{
  if (m_pTimerReconnect)
    m_pTimerReconnect->stop();
  
  stop();
  loadUrl(QUrl(sWbm));
}

void WebView::onReconnect()
{
  //qDebug() << "onReconnect";
  stop();
  reload();
}

void WebView::onCheckError500InternalServerError()
{
  //500 - Internal Server Error
  page()->toPlainText([this](QString sPlain)
  {
    //qDebug() << "onCheckError500InternalServerError " << sPlain.length();
    if (sPlain.length() < 256)
    {
      for (int i=0; i < g_webengine.slError5Text.count(); i++)
      {
        if (sPlain.contains(g_webengine.slError5Text.at(i), Qt::CaseInsensitive) == true)
        {
          //qDebug() << sPlain;
          QTimer::singleShot(200, this, SLOT(onShowReconnectPage()));
          break;
        }
      }
    }
  });
}

void WebView::onShowReconnectPage()
{
  //qDebug() << "onShowReconnectPage";

  if (IsCurrentWebView() ==  false)
  {
    //do not show reconnect page
    return;
  }

  //only in visible BrowserTab
  if (m_pTimerReconnect)
  {
    m_pTimerReconnect->stop();
    m_pTimerReconnect->setSingleShot(true);
    m_pTimerReconnect->start(g_webengine.iReconnectInterval * 1000);

    //close virtual keyboard if neccessary
    //char * pCmd = "close\n";
    //Write2PipedFifo(DEV_VIRTUALKEYBOARD, pCmd);

    //only show reconnect screen in active tab
    reconnectPage.Countdown(g_webengine.iReconnectInterval);
    reconnectPage.Show();
    QString s = g_webengine.sDisconnectedText;
    reconnectPage.SetLabelText(s.replace("%U", url().toString()));
  }
}

QString WebView::GetLocalIpAddress(QString sAdapter)
{
  QString sRet = "";
  try
  {
    QNetworkInterface iface = QNetworkInterface::interfaceFromName(sAdapter);
    if (iface.isValid() == true)
    {
      QList<QNetworkAddressEntry> entries = iface.addressEntries();
      if (!entries.isEmpty()) {
        QNetworkAddressEntry entry = entries.first();
        sRet = entry.ip().toString().trimmed();
      }
    }
  }
  catch(...)
  {
    qDebug() << "Exception in GetLocalIpAddress";
  }
  return sRet;
}

bool WebView::IsLocalhost()
{
  if ( (url().host().contains("localhost")) || (url().host().contains("127.0.0.")) )
  {
    return true;
  }
  else
  {
    QString br0 = GetLocalIpAddress("br0");
    if ((br0.length() > 2 ) && (url().host().contains(br0)))
      return true;

    QString br1 = GetLocalIpAddress("br1");
    if ((br1.length() > 2 ) && (url().host().contains(br1)))
      return true;

    return false;
  }
}

bool WebView::IsLocalhost(QUrl u)
{
  if ( (u.host().contains("localhost")) || (u.host().contains("127.0.0.")) )
  {
    return true;
  }
  else
  {
    QString br0 = GetLocalIpAddress("br0");
    if ((br0.length() > 2 ) && (u.host().contains(br0)))
      return true;

    QString br1 = GetLocalIpAddress("br1");
    if ((br1.length() > 2 ) && (u.host().contains(br1)))
      return true;

    return false;
  }
}

int WebView::GetConnectionState()
{
  int conn_state = CONN_UNDEFINED;
  QByteArray baHost;
  baHost.append(url().host());
  int iPort = url().port();

  if (iPort < 0)
  {
    if (url().scheme().compare("http", Qt::CaseInsensitive) == 0)
    {
      iPort = 80;
    }
    else if (url().scheme().compare("https", Qt::CaseInsensitive) == 0)
    {
      iPort = 443;
    }
  }

  if ((iPort < 0) || (baHost.length() < 3))
    return conn_state;

  if (IsLocalhost())
  {
    return CONN_SUCCESS;
  }

  if (check_network_connection(baHost.data(), iPort) == 0)
  {
    conn_state = CONN_SUCCESS;
  }
  else
  {
    conn_state = CONN_FAILED;
  }

  return conn_state;
}

void WebView::onCheckVisuConnection()
{
  //qDebug() << "onCheckVisuConnection";

  if (IsCurrentWebView())
  {
     if (IsMonitoringConfigured() == false)
     {
       return;
     }
     //reconnect check only in the visible Tab not in other Tabs
     bool bCheck = true;
     struct timespec timeNow;
     int iSeconds = 0;
     if (m_iTcpRxTimestamp > 0)
     {
       clock_gettime(CLOCK_MONOTONIC,&timeNow);
       iSeconds = (int)timeNow.tv_sec - m_iTcpRxTimestamp;
       //qDebug() << "iSeconds: " << iSeconds;
       if (iSeconds <= g_webengine.iReconnectInterval +1)
       {
          //do not check until timeout reached
          bCheck = false;
       }
     }

     if (bCheck)
     {       
       if (GetConnectionState() == CONN_FAILED)
       {
         //show reconnect page with countdown
         if (m_pTimerCommCheck)
         {
           m_pTimerCommCheck->stop();
           m_iTcpRxTimestamp = 0;
         }
         QTimer::singleShot(200, this, SLOT(onShowReconnectPage()));
       }
     }
  }
  else
  {
      if (m_pTimerCommCheck)
      {
        m_pTimerCommCheck->stop();
      }
  }
}

void WebView::SetTcpRxTimestamp(QString sIp, int iTimestamp)
{
  if (url().host().compare(sIp, Qt::CaseInsensitive) == 0)
  {
    m_iTcpRxTimestamp = iTimestamp;
    //qDebug() << "m_iTcpRxTimestamp: " << m_iTcpRxTimestamp;
  }
}

void WebView::RemoveMonitorTmpFile()
{
  try
  {
    QFile file(FILE_TMP_BROWSER_URL);
    file.remove();
  }
  catch(...)
  {
    qDebug() << "Exception in RemoveMonitorTmpFile";
  }
}

void WebView::CreateMonitorTmpFile()
{
  try
  {
    QString sHost = url().host();
    //read first
    QFile f(FILE_TMP_BROWSER_URL);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
       QTextStream in(&f);
       QString sTxt = in.readAll();
       f.close();

       if (sHost.compare(sTxt.trimmed(), Qt::CaseInsensitive) == 0)
       {
         return;
       }
    }

    //write
    QFile file(FILE_TMP_BROWSER_URL);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
       QTextStream out(&file);
       out << sHost;
       file.flush();
       file.close();
    }
  }
  catch(...)
  {
    qDebug() << "Exception in CreateMonitorTmpFile";
  }
}

bool WebView::IsCurrentWebView()
{
  bool bRet = false;
  try
  {
    QObject * pParentObj = parentWidget()->parent();
    TabWidget * pTabs = qobject_cast<TabWidget*>(pParentObj);
    if (pTabs)
    {
      int iCurrentIndex = pTabs->currentIndex();
      int iTabIndex = pTabs->webViewIndex(this);
      if ((iCurrentIndex >= 0) && (iCurrentIndex == iTabIndex))
        bRet = true;
    }
  }
  catch(...)
  {
    qDebug() << "Exception in IsCurrentWebView";
  }
  return bRet;
}

bool WebView::IsMonitoringConfigured()
{
  bool bRet = false;

  try
  {
    QString sCurrentHost = url().host(QUrl::FullyEncoded);
    QString sCurrentPath = url().path(QUrl::FullyEncoded);
    if (sCurrentPath.endsWith("/"))
    {
      sCurrentPath = sCurrentPath.left(sCurrentPath.length()-1);
    }

    for (int i=0; i < g_webengine.slUrls.count(); i++)
    {
      QUrl u(g_webengine.slUrls.at(i));
      QString sHost = u.host(QUrl::FullyEncoded);
      QString sPath = u.path(QUrl::FullyEncoded);
      QString m = g_webengine.slMonitor.at(i);

      if (sPath.endsWith("/"))
      {
        sPath = sPath.left(sPath.length()-1);
      }

      //host match
      if (sCurrentHost.compare(sHost, Qt::CaseInsensitive) == 0)
      {
        //check exact path
        if (sCurrentPath.compare(sPath, Qt::CaseInsensitive) == 0)
        {
          //exact path equal
          if (m.compare("1") == 0)
          {
            bRet = true;
          }
          else
          {
            bRet = false;
          }
          break;
        }

        //check shortened path
        QString sCurrentPathShortened = sCurrentPath.left(sPath.length());
        if (sCurrentPathShortened.compare(sPath, Qt::CaseInsensitive) == 0)
        {
          //shortened path equal
          if (m.compare("1") == 0)
          {
            bRet = true;
          }
        }

      }
    }
  }
  catch(...)
  {
    qDebug() << "Exception in IsMonitoringConfigured";
  }

  return bRet;
}

