///------------------------------------------------------------------------------
/// \file    keyboard.cpp
///
/// \brief   platform input context
///
/// \author  WRü: elrest Automationssysteme GmbH
///------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "keyboard.h"

#include <QCoreApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>

#include <QGridLayout>
#include <QSignalMapper>
#include <QPushButton>

#include <QDBusConnection>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variable definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

template <class T> class VPtr {
public:

  static T* asPtr(QVariant v) {
      return (T *) v.value<void *>();
  }

  static QVariant asQVariant(T* ptr) {
      return qVariantFromValue((void *) ptr);
  }
};

int g_InputMethodHints = 0;

//Keyboard::Keyboard()
Keyboard::Keyboard(QWidget *parent):QWidget(parent)
{
  setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::BypassWindowManagerHint);

  inputPanel = NULL;
  pCmdThread = NULL;
  bEnabled = true;
  m_af = Qt::AlignBottom;

  Initialize();
}

Keyboard::~Keyboard()
{

  VirtualKeyb * ptr;
  for (int i = 0; i < m_panelList.count(); i++)
  {
    ptr = m_panelList.at(i);
    if (ptr)
      delete ptr;
  }
  m_panelList.clear();
}



/// \brief enable / disable virtual keyboard
/// \param[in]  bValue enablde, disabled
///
void Keyboard::SetEnabled(bool bValue)
{
  bEnabled = bValue;
}

/// \brief get virtual keyboard enabled state
/// \retval true  enabled
/// \retval false  disabled
///
bool Keyboard::GetEnabled()
{
  return bEnabled;
}

/// \brief initialize virtual keyboard context
/// parse xml file, read keyboard panels and layout
///
void Keyboard::Initialize()
{  
  parseXML();

  if ((m_panelList.count() > 1) && (inputPanel))
  {
    inputPanel = m_panelList.at(0);
  }
  
  startCmdThread();
}

/*
/// \brief get ident string / name
///
QString Keyboard::identifierName()
{
  return "VirtualKeybContext";
}
*/

/// \brief update position
///
void Keyboard::updatePosition()
{

  int ypos = QCursor::pos().y();

  if (inputPanel)
  {

    QRect kRect = inputPanel->rect();
    QSize newSize(kRect.width(), kRect.height());
    if (ypos > (inputPanel->m_iScreenHeight / 2))
    {
      m_af = Qt::AlignTop;
    }
    else
    {
      m_af = Qt::AlignBottom;
    }

    if (m_af != inputPanel->m_af)
    {
      inputPanel->SetPanelDimensions(newSize);
    }

    inputPanel->m_af = m_af;
  }

}

/// \brief handle swap button
/// switch between configured panels
///
void Keyboard::HandleSwap()
{
  int count = m_panelList.count();
  if (count > 1)
  {

    for (int k = 0; k < count; k++)
    {
      if (inputPanel == m_panelList.at(k))
      {
        VirtualKeyb * savePtr = inputPanel;

        //SWAP
        if (k == (count - 1))
        {
          inputPanel = m_panelList.at(0);
        }
        else
        {
          inputPanel = m_panelList.at(k + 1);
        }

        QRect kRect = inputPanel->rect();
        QSize newSize(kRect.width(), kRect.height());
        inputPanel->SetPanelDimensions(newSize);

        savePtr->m_bSwapButtonCalled = true;
        inputPanel->m_bSwapButtonCalled = true;

        inputPanel->Show();       
        savePtr->Hide(true);

        break;
      }

    }
  }

}

/// \brief parse virtualkeyboard.xml file
/// read panels, layout, key definitions
///
void Keyboard::parseXML()
{
  QRect rScreen = QApplication::desktop()->screenGeometry();
  QString vkXmlFileName;

  //f.e. virtualkeyboard_272x480.xml
  vkXmlFileName="/etc/specific/virtualkeyboard_" + QString::number(rScreen.width()) + "x" + QString::number(rScreen.height())  + ".xml";
  QFile* file = new QFile(vkXmlFileName);
  if (!file)
  {
    //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    return;
  }

  if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
  {
    //use default file: virtualkeyboard.xml
    file->close();
    delete file;
    file = new QFile("/etc/specific/virtualkeyboard.xml");
    if (!file)
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
      return;
    }

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: could not read virtual keyboard xml file");
      return;
    }
  }
  QXmlStreamReader xml(file);

  while (!xml.atEnd() && !xml.hasError())
  {
    /* Read next element.*/
    QXmlStreamReader::TokenType token = xml.readNext();
    /* If token is just StartDocument, we'll go to next.*/
    if (token == QXmlStreamReader::StartDocument)
    {
      continue;
    }
    /* If token is StartElement, we'll see if we can read it.*/
    if (token == QXmlStreamReader::StartElement)
    {

      if (xml.name() == "layouts")
      {
        continue;
      }

      if (xml.name() == "layout")
      {
        parseLayout(xml);
      }
    }
  }
  /* Error handling. */
  if (xml.hasError())
  {
    qDebug() << "webkit: " << "xml parse errors";
  }
  /* Removes any device() or data from the reader
   * and resets its internal state to the initial state. */
  xml.clear();

  if (file)
  {
    file->close();
    delete file;
  }

}

/// \brief parse layout from xml file
///
void Keyboard::parseLayout(QXmlStreamReader &xml)
{

  if (xml.tokenType() != QXmlStreamReader::StartElement
      && xml.name() == "layout")
  {
    return;
  }

  inputPanel = new VirtualKeyb;
  if (inputPanel)
  {
#ifdef DEBUG_MSG
    qDebug() << "webkit: " << "parseLayout new inputPanel " << inputPanel;
#endif
    m_panelList.append(inputPanel);
    //inputPanel->Initialize();

    connect(inputPanel, SIGNAL(vkKeyClicked(QString)), this, SLOT(slotKeyClicked(QString)));
    connect(inputPanel, SIGNAL(vkSpecialKeyClicked(int)), this, SLOT(slotSpecialKeyClicked(int)));

    connect(this, SIGNAL(textChanged(QString, int)), inputPanel, SLOT(slotTextChanged(QString, int)));

  }
  else
  {
    //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    return;
  }

  // Next element...
  xml.readNext();

  QString s;

  while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "layout"))
  {
    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "repetition_rate")
    {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::Characters)
      {
        inputPanel->m_iRepetitionRate = xml.text().toString().toInt();
      }
    }
    //else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "vpixels_limit_textfield")
    //{
    //  xml.readNext();
    //  if (xml.tokenType() == QXmlStreamReader::Characters)
    //  {
    //    inputPanel->m_iFreeVPixels = xml.text().toString().toInt();
    //  }
    //}
    else if (xml.tokenType() == QXmlStreamReader::StartElement  && xml.name() == "font")
    {
      QFont fnt = inputPanel->font();

      //read font values
      xml.readNext();
      while (!(xml.tokenType() == QXmlStreamReader::EndElement
          && xml.name() == "font"))
      {
        if (xml.name() == "family" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            //s = xml.text().toString();
            fnt.setFamily(xml.text().toString());
          }
        }
        else if (xml.name() == "bold" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            if (xml.text().toString() == "true")
              fnt.setBold(true);
            else
              fnt.setBold(false);
          }
        }
        else if (xml.name() == "italic" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            if (xml.text().toString() == "true")
              fnt.setItalic(true);
            else
              fnt.setItalic(false);
          }
        }
        else if (xml.name() == "minpointsize" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            s = xml.text().toString();
            inputPanel->m_iMinFontSize = s.toInt();
          }
        }
        else if (xml.tokenType() == QXmlStreamReader::Invalid)
        {
          break;
        }
        xml.readNext();
      }

      int iFontSize = 10;
      if (inputPanel->m_iScreenWidth <= 272)
      {
        iFontSize = 10;
      }
      else if (inputPanel->m_iScreenWidth <= 480)
      {
        iFontSize = 12;
      }
      else if (inputPanel->m_iScreenWidth <= 640)
      {
        iFontSize = 12;
      }
      else if (inputPanel->m_iScreenWidth <= 800)
      {
        iFontSize = 14;
      }
      else if (inputPanel->m_iScreenWidth <= 1280)
      {
        iFontSize = 16;
      }
      else if (inputPanel->m_iScreenWidth <= 1920)
      {
        iFontSize = 18;
      }


      fnt.setPointSize(qMax(iFontSize, inputPanel->m_iMinFontSize));
      inputPanel->setFont(fnt);
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "style")
    {
      //read style values
      xml.readNext();
      while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "style"))
      {
        if (xml.name() == "minbtnwidth" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            s = xml.text().toString();
            inputPanel->m_iMinBtnWidth = s.toInt();
          }
        }
        else if (xml.name() == "minbtnheight" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            s = xml.text().toString();
            inputPanel->m_iMinBtnHeight = s.toInt();
          }
        }
        else if (xml.name() == "qt_style_window" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            inputPanel->m_sWndStyle = xml.text().toString();
            inputPanel->setStyleSheet(inputPanel->m_sWndStyle);
          }
        }
        else if (xml.name() == "qt_style_button" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            inputPanel->m_sBtnStyle = xml.text().toString();
          }
        }
        else if (xml.name() == "qt_style_button_control" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            inputPanel->m_sBtnStyleControl = xml.text().toString();
          }
        }
        else if (xml.name() == "rowcount" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            s = xml.text().toString();
            inputPanel->m_iRowCount = s.toInt();
          }
        }
        else if (xml.name() == "colcount" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            s = xml.text().toString();
            inputPanel->m_iColCount = s.toInt();
          }
        }
        else if (xml.name() == "hspace" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            s = xml.text().toString();
            inputPanel->m_hspace = s.toInt();
          }
        }
        else if (xml.name() == "vspace" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          xml.readNext();
          if (xml.tokenType() == QXmlStreamReader::Characters)
          {
            s = xml.text().toString();
            inputPanel->m_vspace = s.toInt();
          }
        }
        else if (xml.tokenType() == QXmlStreamReader::Invalid)
        {
          break;
        }
        xml.readNext();
      }
    }
    else if (xml.tokenType() == QXmlStreamReader::StartElement
        && xml.name() == "rows")
    {
      parseRows(xml);
    }
    else if (xml.tokenType() == QXmlStreamReader::Invalid)
    {
      break;
    }

    // ...and next...
    xml.readNext();
  }
  return;
}

/// \brief parse rows from xml file
///
void Keyboard::parseRows(QXmlStreamReader &xml)
{
  int max_xpos = 0;
  QString s;
  int iLineWidth = 0;

  inputPanel->SetBtnDimensions();

  //read style values
  xml.readNext();
  while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "rows"))
  {
    if (xml.name() == "row" && xml.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = xml.attributes();

      xml.readNext();
      while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "row"))
      {
        if (xml.name() == "key" && xml.tokenType() == QXmlStreamReader::StartElement)
        {
          tKeyValues kv;
          QXmlStreamAttributes attributes = xml.attributes();

          kv.widthscale = 1.000;
          if (attributes.hasAttribute("widthscale"))
          {
            s = attributes.value("widthscale").toString();
            kv.widthscale = s.toDouble();
          }

          xml.readNext();
          while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "key"))
          {
            if (xml.name() == "char" && xml.tokenType() == QXmlStreamReader::StartElement)
            {
              xml.readNext();
              if (xml.tokenType() == QXmlStreamReader::Characters)
              {
                kv.sKey = xml.text().toString();
              }
            }
            else if (xml.name() == "code" && xml.tokenType() == QXmlStreamReader::StartElement)
            {
              xml.readNext();
              if (xml.tokenType() == QXmlStreamReader::Characters)
                kv.sCode = xml.text().toString();
            }
            else if (xml.name() == "char_shift" && xml.tokenType() == QXmlStreamReader::StartElement)
            {
              xml.readNext();
              if (xml.tokenType() == QXmlStreamReader::Characters)
              {
                kv.sKeyShifted = xml.text().toString();
              }
            }
            else if (xml.name() == "code_shift" && xml.tokenType() == QXmlStreamReader::StartElement)
            {
              xml.readNext();
              if (xml.tokenType() == QXmlStreamReader::Characters)
                kv.sCodeShifted = xml.text().toString();
            }
            else if (xml.name() == "icon" && xml.tokenType() == QXmlStreamReader::StartElement)
            {
              xml.readNext();
              if (xml.tokenType() == QXmlStreamReader::Characters)
                kv.sIcon = xml.text().toString();
            }
            else if (xml.name() == "autorepeat" && xml.tokenType() == QXmlStreamReader::StartElement)
            {
              xml.readNext();
              if (xml.tokenType() == QXmlStreamReader::Characters)
                kv.sAutoRepeat = xml.text().toString();
            }
            else if (xml.name() == "control_text" && xml.tokenType() == QXmlStreamReader::StartElement)
            {
              xml.readNext();
              if (xml.tokenType() == QXmlStreamReader::Characters)
                kv.sControlText = xml.text().toString();
            }
            else if (xml.tokenType() == QXmlStreamReader::Invalid)
            {
              break;
            }

            xml.readNext();
          }

          iLineWidth += (inputPanel->m_iBtnWidth * kv.widthscale);
          CreateButton(&kv, inputPanel->m_btn_xpos, inputPanel->m_btn_ypos);

        }
        xml.readNext();
      }

      //new line
      inputPanel->m_btn_ypos += inputPanel->m_iBtnHeight + inputPanel->m_vspace;
      max_xpos = qMax(max_xpos, inputPanel->m_btn_xpos);
      inputPanel->m_btn_xpos = 0;
      iLineWidth = 0;


    }
    xml.readNext();
  }

  max_xpos = qMin(max_xpos, inputPanel->m_iScreenWidth);

  QSize newSize(max_xpos, qMin(inputPanel->m_btn_ypos, inputPanel->m_iScreenHeight));
  inputPanel->SetPanelDimensions(newSize);
}

/// \brief create key button
///
void Keyboard::CreateButton(tKeyValues * ptr, int &xpos, int &ypos)
{
  //qDebug() << "webkit: " << "CreateButton " << ptr->sKey << " x: " << xpos << " y: " << ypos ;
  if (ptr)
  {
    if (ptr->sKey.at(0) == '{')
    {
      CreateSpecialButton(ptr, xpos, ypos);
    }
    else
    {
      inputPanel->m_pBtn = new CVKButton(ptr->sKey, inputPanel);
      if (inputPanel->m_pBtn)
      {
        inputPanel->m_pBtn->setFocusPolicy(Qt::NoFocus);
        inputPanel->m_pBtn->setAutoDefault(false);

        inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
        inputPanel->m_pBtn->m_sKey = ptr->sKey;
        inputPanel->m_pBtn->m_sKeyShifted = ptr->sKeyShifted;
        inputPanel->m_pBtn->m_iCode = ptr->sCode.toInt();
        inputPanel->m_pBtn->m_iCodeShifted = ptr->sCodeShifted.toInt();

        inputPanel->m_pBtn->setFont(inputPanel->font());
        inputPanel->m_btnList.append(inputPanel->m_pBtn);

        if (ptr->sAutoRepeat == "1")
        {
          inputPanel->m_pBtn->m_iRepetitionAllowed = 1;
        }

        QSize btnSize(inputPanel->m_iBtnWidth, inputPanel->m_iBtnHeight);

        // set size and location of the button
        inputPanel->m_pBtn->setGeometry(QRect(QPoint(xpos, ypos), btnSize));

        if ((ptr->sKey.length() > 0) || (ptr->sKeyShifted.length() > 0) || (ptr->sCode.length() > 0) || (ptr->sCodeShifted.length() > 0))
        {
          // Connect button signal to appropriate slot
          connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(OnButtonPressed()));
          connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
        }

        xpos += (inputPanel->m_hspace);
        xpos += (inputPanel->m_iBtnWidth);

      }
      else
      {
        //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
      }
    }
  }

}

/// \brief create special key button
/// like shift, enter, backspacet etc.
///
void Keyboard::CreateSpecialButton(tKeyValues * pKeyVal, int &xpos,
    int &ypos)
{
  inputPanel->m_pBtn = NULL;
  QString sText;
  int iWidth = inputPanel->m_iBtnWidth * pKeyVal->widthscale;
  int iQtKeyConst = 0;

  if (pKeyVal->widthscale > 1.000)
  {
    iWidth += ((pKeyVal->widthscale - 1.000) * inputPanel->m_hspace);
  }

  if (pKeyVal->sKey == "{tab}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Tab;
      //inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyleControl);
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleTab()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{enter}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Enter; //Key_Enter
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle); //m_sBtnStyleControl);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleEnter()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{bksp}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Backspace;
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleBksp()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{clear}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleClear()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{space}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Space;
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleSpace()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{esc}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Escape;
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleEsc()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{shift}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Shift;
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleShift()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{shiftlock}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleShiftLock()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{left}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Left;
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleLeft()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{right}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Right;
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleRight()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{up}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Up;
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleUp()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{down}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      iQtKeyConst = Qt::Key_Down;
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(pressed()), inputPanel, SLOT(HandleDown()));
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{ok}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyleControl);
      connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(HandleOk()));
      //connect(inputPanel->m_pBtn, SIGNAL(released()), inputPanel, SLOT(OnButtonReleased()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }
  else if (pKeyVal->sKey == "{swap}")
  {
    sText = pKeyVal->sControlText;
    inputPanel->m_pBtn = new CVKButton(sText, inputPanel);
    if (inputPanel->m_pBtn)
    {
      inputPanel->m_pBtn->setStyleSheet(inputPanel->m_sBtnStyle);
      connect(inputPanel->m_pBtn, SIGNAL(released()), this, SLOT(HandleSwap()));
    }
    else
    {
      //setRgbLed(RGB_LED_STATE_RE_BLINK, "webkit: not enough memory");
    }
  }

  if (inputPanel->m_pBtn)
  {
    inputPanel->m_pBtn->m_iSpecialBtn = 1;
    inputPanel->m_pBtn->setFocusPolicy(Qt::NoFocus);
    inputPanel->m_pBtn->setAutoDefault(false);

    inputPanel->m_pBtn->m_sKey = pKeyVal->sKey;
    inputPanel->m_pBtn->m_sKeyShifted = pKeyVal->sKeyShifted;
    inputPanel->m_pBtn->m_iCode = pKeyVal->sCode.toInt();
    inputPanel->m_pBtn->m_iCodeShifted = pKeyVal->sCodeShifted.toInt();

    if (iQtKeyConst)
    {
      if (inputPanel->m_pBtn->m_iCode == 0)
        inputPanel->m_pBtn->m_iCode = iQtKeyConst;

      if (inputPanel->m_pBtn->m_iCodeShifted == 0)
        inputPanel->m_pBtn->m_iCodeShifted = iQtKeyConst;
    }

    inputPanel->m_pBtn->setFont(inputPanel->font());
    inputPanel->m_btnListSpecial.append(inputPanel->m_pBtn);

    if (!pKeyVal->sIcon.isEmpty())
    {
      inputPanel->m_pBtn->setText("");
      QIcon icon(pKeyVal->sIcon);
      inputPanel->m_pBtn->setIcon(icon);
      inputPanel->m_pBtn->setIconSize( QSize(inputPanel->m_pBtn->size().width() - 6, inputPanel->m_pBtn->size().height() - 4));
    }

    if (pKeyVal->sAutoRepeat == "1")
    {
      inputPanel->m_pBtn->m_iRepetitionAllowed = 1;
    }

    inputPanel->m_pBtn->setGeometry(QRect(QPoint(xpos, ypos), QSize(iWidth, inputPanel->m_iBtnHeight)));

    xpos += iWidth;
    xpos += (inputPanel->m_hspace);

  }
}

/// \brief show software input panel
///
void Keyboard::showKeyboard()
{
  if (inputPanel)
  {
    //signal from dbus
    inputPanel->Show();
  }
}

/// \brief hide software input panel
///
void Keyboard::hideKeyboard()
{
  if (inputPanel)
  {
    inputPanel->Hide();
  }
}

bool Keyboard::keyboardVisible() const
{
  if (inputPanel)
  {
    return inputPanel->isVisible();
  }
  else
  {
    return false;
  }
}

void Keyboard::slotOpenFromPlugin()
{
  //open from webenginebrowser - not targetvisu
  if (inputPanel)
  {
    //inputPanel->Log("slotOpenFromPlugin");
    inputPanel->m_iTvOpen = 0;
    inputPanel->m_bPluginOpen = true;
  }
}

void Keyboard::slotSpecialKeyClicked(int key)
{
  emit specialKeyClicked(key);
}

void Keyboard::slotKeyClicked(const QString &text)
{
   emit keyClicked(text);
}


void Keyboard::inputFieldTextChanged(const QString &s, int iCursorPos)
{

  emit textChanged(s, iCursorPos);

}

/// \brief starting cmd thread
///
void Keyboard::startCmdThread()
{
  //call only once

  if (pCmdThread)
    return;

  pCmdThread = new cmdThread(this);

  if (! pCmdThread)
    return;

  connect(pCmdThread,
          SIGNAL(cmdSignalReceived(QString)),
          this,
          SLOT(cmdSlotReceived(QString)) );

  //start thread function run
  pCmdThread->start(QThread::NormalPriority);

}

/// \brief cmd message string received
///
void Keyboard::cmdSlotReceived(QString s)
{
  //qDebug() << "cmdSlotReceived: " << s;

  if (s.left(4).compare("open", Qt::CaseInsensitive) == 0)
  {
    //open full
    inputPanel->Show();
  }
  else if (s.left(6).compare("tvopen", Qt::CaseInsensitive) == 0)
  {
    if ((inputPanel) && (inputPanel->isVisible() == false))
    {
      inputPanel->m_iTvOpen++;
      if (inputPanel->m_iTvOpen == 1)
      {
        //inputPanel->Log("tvopen");
        emit signalShowVirtualKeyboardFromTv();
      }
    }
  }
  else if (s.left(5).compare("close", Qt::CaseInsensitive) == 0)
  {
    //close
    inputPanel->m_iTvOpen = 0;
    inputPanel->Hide();
  }
  else if (s.left(6).compare("enable", Qt::CaseInsensitive) == 0)
  {
    //enable
    //inputPanel->SetVkEnabled(true);
    VirtualKeyb * ptr;
    for (int i = 0; i < m_panelList.count(); i++)
    {
      ptr = m_panelList.at(i);
      ptr->SetVkEnabled(true);
    }

  }
  else if (s.left(7).compare("disable", Qt::CaseInsensitive) == 0)
  {
    //disable
    //inputPanel->SetVkEnabled(false);

    VirtualKeyb * ptr;
    for (int i = 0; i < m_panelList.count(); i++)
    {
      ptr = m_panelList.at(i);
      ptr->SetVkEnabled(false);
    }
  }
}

