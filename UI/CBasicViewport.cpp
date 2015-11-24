#include "CBasicViewport.h"
#include <Core/CDrawUtil.h>
#include <Core/CGraphics.h>
#include <Common/Math.h>
#include <GL/glew.h>

CBasicViewport::CBasicViewport(QWidget *pParent) :
    QOpenGLWidget(pParent),
    mLastDrawTime(CTimer::GlobalTime()),
    mKeysPressed(0),
    mButtonsPressed(0),
    mCursorState(Qt::ArrowCursor),
    mCursorVisible(true)
{
    setMouseTracking(true);
    mCamera.SetAspectRatio((float) width() / height());
}

CBasicViewport::~CBasicViewport()
{
}

void CBasicViewport::initializeGL()
{
    // Initialize CGraphics
    CGraphics::Initialize();

    // Setting various GL flags
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.f, 5.f);

    // Clear cached material
    CMaterial::KillCachedMaterial();
    CShader::KillCachedShader();

    // Initialize size
    OnResize();
}

void CBasicViewport::paintGL()
{
    // Prep render
    glViewport(0, 0, width(), height());
    glLineWidth(1.f);
    glEnable(GL_DEPTH_TEST);

    // Actual rendering is intended to be handled by subclassing CBasicViewport and
    // reimplementing Render().
    Paint();

    // Finally, draw XYZ axes in the corner
    DrawAxes();
}

void CBasicViewport::resizeGL(int w, int h)
{
    mCamera.SetAspectRatio((float) w / h);
    glViewport(0, 0, w, h);
    OnResize();
}

void CBasicViewport::mousePressEvent(QMouseEvent *pEvent)
{
    setFocus();

    if (pEvent->button() == Qt::MidButton)   mButtonsPressed |= eMiddleButton;
    if (pEvent->button() == Qt::RightButton) mButtonsPressed |= eRightButton;

    if (IsMouseInputActive())
    {
        SetCursorVisible(false);
        mMouseMoved = false;
        mMoveTimer.Restart();
    }

    // Left click only activates if mouse input is inactive to prevent the user from
    // clicking on things and creating selection rectangles while the cursor is hidden
    else
    {
        if (pEvent->button() == Qt::LeftButton)
            mButtonsPressed |= eLeftButton;

        OnMouseClick(pEvent);
    }

    mLastMousePos = pEvent->globalPos();
}

void CBasicViewport::mouseReleaseEvent(QMouseEvent *pEvent)
{
    bool fromMouseInput = IsMouseInputActive();
    if (pEvent->button() == Qt::LeftButton)  mButtonsPressed &= ~eLeftButton;
    if (pEvent->button() == Qt::MidButton)   mButtonsPressed &= ~eMiddleButton;
    if (pEvent->button() == Qt::RightButton) mButtonsPressed &= ~eRightButton;

    // Make cursor visible if needed
    if (!IsMouseInputActive())
        SetCursorVisible(true);

    // Run mouse release if we didn't just exit mouse input (or regardless on left click)
    if (!fromMouseInput || (pEvent->button() == Qt::LeftButton))
        OnMouseRelease(pEvent);
}

void CBasicViewport::mouseMoveEvent(QMouseEvent* /*pEvent*/)
{
    // todo: draggable selection rectangle
}

void CBasicViewport::wheelEvent(QWheelEvent *pEvent)
{
    // Maybe track a "wheel delta" member variable and let CCamera decide what to do with it?
    mCamera.Zoom(pEvent->angleDelta().y() / 6000.f);
}

void CBasicViewport::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Q: mKeysPressed |= eQKey; break;
    case Qt::Key_W: mKeysPressed |= eWKey; break;
    case Qt::Key_E: mKeysPressed |= eEKey; break;
    case Qt::Key_A: mKeysPressed |= eAKey; break;
    case Qt::Key_S: mKeysPressed |= eSKey; break;
    case Qt::Key_D: mKeysPressed |= eDKey; break;
    case Qt::Key_Control: mKeysPressed |= eCtrlKey; break;
    }
}

void CBasicViewport::keyReleaseEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
    case Qt::Key_Q: mKeysPressed &= ~eQKey; break;
    case Qt::Key_W: mKeysPressed &= ~eWKey; break;
    case Qt::Key_E: mKeysPressed &= ~eEKey; break;
    case Qt::Key_A: mKeysPressed &= ~eAKey; break;
    case Qt::Key_S: mKeysPressed &= ~eSKey; break;
    case Qt::Key_D: mKeysPressed &= ~eDKey; break;
    case Qt::Key_Control: mKeysPressed &= ~eCtrlKey; break;
    }
}

void CBasicViewport::focusOutEvent(QFocusEvent*)
{
    // When the widget loses focus, release all input.
    mButtonsPressed = 0;
    mKeysPressed = 0;
    SetCursorVisible(true);
}

void CBasicViewport::contextMenuEvent(QContextMenuEvent *pEvent)
{
    // Only allow context menu if we aren't exiting mouse input mode.
    if (!mMouseMoved && (mMoveTimer.Time() < 0.5))
        ContextMenu(pEvent);
}

void CBasicViewport::SetCursorState(const QCursor &Cursor)
{
    mCursorState = Cursor;

    if (IsCursorVisible())
        setCursor(Cursor);
}

void CBasicViewport::SetCursorVisible(bool visible)
{
    mCursorVisible = visible;

    if (visible)
        setCursor(mCursorState);
    else
        setCursor(Qt::BlankCursor);
}

bool CBasicViewport::IsCursorVisible()
{
    return mCursorVisible;
}

bool CBasicViewport::IsMouseInputActive()
{
    static const int skMoveButtons = eMiddleButton | eRightButton;
    return ((mButtonsPressed & skMoveButtons) != 0);
}

bool CBasicViewport::IsKeyboardInputActive()
{
    static const int skMoveKeys = eQKey | eWKey | eEKey | eAKey | eSKey | eDKey;
    return ((mKeysPressed & skMoveKeys) != 0);
}

CCamera& CBasicViewport::Camera()
{
    return mCamera;
}

CRay CBasicViewport::CastRay()
{
    CVector2f MouseCoords = MouseDeviceCoordinates();
    return mCamera.CastRay(MouseCoords);
}

CVector2f CBasicViewport::MouseDeviceCoordinates()
{
    QPoint MousePos = QCursor::pos();
    QPoint ThisPos = this->mapToGlobal(pos());
    MousePos -= ThisPos;

    CVector2f Device(
        (((2.f * MousePos.x()) / width()) - 1.f),
        (1.f - ((2.f * MousePos.y()) / height()))
    );
    return Device;
}

double CBasicViewport::LastRenderDuration()
{
    return mFrameTimer.Time();
}

// ************ PUBLIC SLOTS ************
void CBasicViewport::ProcessInput()
{
    // Process camera input
    double DeltaTime = CTimer::GlobalTime() - mLastDrawTime;
    mLastDrawTime = CTimer::GlobalTime();

    if (IsMouseInputActive())
    {
        float XMovement = (QCursor::pos().x() - mLastMousePos.x()) * 0.01f;
        float YMovement = (QCursor::pos().y() - mLastMousePos.y()) * 0.01f;

        if ((XMovement != 0) || (YMovement != 0))
        {
            mCamera.ProcessMouseInput((EKeyInputs) mKeysPressed, (EMouseInputs) mButtonsPressed, XMovement, YMovement);
            QCursor::setPos(mLastMousePos);
            mMouseMoved = true;
        }
    }

    if (IsKeyboardInputActive())
        mCamera.ProcessKeyInput((EKeyInputs) mKeysPressed, DeltaTime);

    // Check user input
    CheckUserInput();
}

void CBasicViewport::Render()
{
    mFrameTimer.Start();
    update();
    mFrameTimer.Stop();
}

// ************ PRIVATE ************
void CBasicViewport::ProcessInput(double /*DeltaTime*/)
{
}

void CBasicViewport::DrawAxes()
{
    // Draw 64x64 axes in lower-left corner with 8px margins
    glViewport(8, 8, 64, 64);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthRange(0.f, 1.f);

    CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(mCamera.Direction() * 5).ToMatrix4f();
    CGraphics::sMVPBlock.ViewMatrix = mCamera.RotationOnlyViewMatrix();
    CGraphics::sMVPBlock.ProjectionMatrix = Math::OrthographicMatrix(-1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);
    CGraphics::UpdateMVPBlock();

    glLineWidth(1.f);
    CDrawUtil::DrawLine(CVector3f(0,0,0), CVector3f(1,0,0), CColor::skRed);   // X
    CDrawUtil::DrawLine(CVector3f(0,0,0), CVector3f(0,1,0), CColor::skGreen); // Y
    CDrawUtil::DrawLine(CVector3f(0,0,0), CVector3f(0,0,1), CColor::skBlue);  // Z
}