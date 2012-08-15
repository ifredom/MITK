/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "TestPerspective2.h"
#include "MinimalView.h"
#include "TestView.h"
#include "berryIFolderLayout.h"

TestPerspective2::TestPerspective2()
{
}

void TestPerspective2::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  //layout->SetFixed(true);
  std::string editorArea = layout->GetEditorArea();
  layout->SetEditorAreaVisible(true);
  layout->AddStandaloneView("org.mitk.views.testview", false, 1, 1.0f, layout->GetEditorArea());
  //layout->AddView("org.mitk.views.testview", 1, 1.0f, editorArea);
  layout->GetViewLayout("org.mitk.views.testview")->SetCloseable(false);
  layout->GetViewLayout("org.mitk.views.testview")->SetMoveable(false);

  ////layout->AddStandaloneView(TestView::VIEW_ID,true,2,0.33f, layout->GetEditorArea());
  //layout->AddView(TestView::VIEW_ID, 2, 0.33f, editorArea);
  //layout->GetViewLayout(TestView::VIEW_ID)->SetCloseable(false);
  

  //berry::IFolderLayout::Pointer topLeft = layout->CreateFolder("topLeft", berry::IPageLayout::LEFT, 0.25f,
  //  editorArea);
  //topLeft->AddView(berry::IPageLayout::ID_RES_NAV);
  //topLeft->AddPlaceholder(berry::IPageLayout::ID_BOOKMARKS);
  //
  //berry::IFolderLayout::Pointer bottomLeft = layout->CreateFolder("bottomLeft", berry::IPageLayout::BOTTOM, 0.50f,
  //   "topLeft");
  //bottomLeft->AddView(berry::IPageLayout::ID_OUTLINE);
  //bottomLeft->AddView(berry::IPageLayout::ID_PROP_SHEET);

 // Bottom right: Task List view
 // layout->AddView(berry::IPageLayout::ID_TASK_LIST, berry::IPageLayout::BOTTOM, 0.66f, editorArea);
 
}
