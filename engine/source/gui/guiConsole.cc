//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "graphics/dgl.h"
#include "gui/guiTypes.h"
#include "gui/guiControl.h"
#include "gui/guiConsole.h"
#include "gui/containers/guiScrollCtrl.h"

IMPLEMENT_CONOBJECT(GuiConsole);

GuiConsole::GuiConsole()
{
   mBounds.extent.set(64, 64);
   mCellSize.set(1, 1);
   mSize.set(1, 0);
}

bool GuiConsole::onWake()
{
   if (! Parent::onWake())
      return false;

   return true;
}

S32 GuiConsole::getMaxWidth(S32 startIndex, S32 endIndex)
{
   //sanity check
   U32 size;
   ConsoleLogEntry *log;

   Con::getLockLog(log, size);

   if(startIndex < 0 || (U32)endIndex >= size || startIndex > endIndex)
      return 0;

   S32 result = 0;
   for(S32 i = startIndex; i <= endIndex; i++)
      result = getMax(result, (S32)(mProfile->getFont(mFontSizeAdjust)->getStrWidth((const UTF8 *)log[i].mString)));
   
   Con::unlockLog();
   
   return(result + 6);
}

void GuiConsole::onPreRender()
{
   //see if the size has changed
   U32 size;
   ConsoleLogEntry *log;

   Con::getLockLog(log, size);
   Con::unlockLog(); // we unlock immediately because we only use size here, not log.

   U32 prevSize = mBounds.extent.y / mCellSize.y;
   if ( prevSize > size )
       prevSize = 0;

   if(size != prevSize)
   {
      //first, find out if the console was scrolled up
      bool scrolled = false;
      GuiScrollCtrl *parent = dynamic_cast<GuiScrollCtrl*>(getParent());

      if(parent)
         scrolled = parent->isScrolledToBottom();

      //find the max cell width for the new entries
      S32 newMax = getMaxWidth(0, size - 1);
      if(newMax > mCellSize.x)
         mCellSize.set(newMax, mProfile->getFont(mFontSizeAdjust)->getHeight());

      //set the array size
      mSize.set(1, size);

      //resize the control
      resize(mBounds.point, Point2I(mCellSize.x, mCellSize.y * size));

      //if the console was scrolled to the bottom then make sure any new row is also visible
      if (scrolled)
         scrollCellVisible(Point2I(0,mSize.y - 1));
   }
}

void GuiConsole::onRenderCell(Point2I offset, Point2I cell, bool selected, bool mouseOver)
{
   U32 size;
   ConsoleLogEntry *log;

   Con::getLockLog(log, size);

   ConsoleLogEntry &entry = log[cell.y];
   switch (entry.mLevel)
   {
      case ConsoleLogEntry::Normal:   dglSetBitmapModulation(mProfile->mFontColor); break;
      case ConsoleLogEntry::Warning:  dglSetBitmapModulation(mProfile->mFontColorHL); break;
      case ConsoleLogEntry::Error:    dglSetBitmapModulation(mProfile->mFontColorNA); break;
      case ConsoleLogEntry::NUM_CLASS: Con::errorf("Unhandled case in GuiConsole::onRenderCell, NUM_CLASS");
   }
   dglDrawText(mProfile->getFont(mFontSizeAdjust), Point2I(offset.x + 3, offset.y), entry.mString, mProfile->mFontColors);
   
   Con::unlockLog();
}
