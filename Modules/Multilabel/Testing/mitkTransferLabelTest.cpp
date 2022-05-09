/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkTransferLabelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkTransferLabelTestSuite);
  MITK_TEST(TestTransfer_defaults);
  MITK_TEST(TestTransfer_Merge_RegardLocks);
  MITK_TEST(TestTransfer_Merge_IgnoreLocks);
  MITK_TEST(TestTransfer_Replace_RegardLocks);
  MITK_TEST(TestTransfer_Replace_IgnoreLocks);
  MITK_TEST(TestTransfer_multipleLabels);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::LabelSetImage::Pointer m_SourceImage;
  mitk::LabelSetImage::PixelType m_LabelLocked1 = 1;
  mitk::LabelSetImage::PixelType m_LabelLocked2 = 2;
  mitk::LabelSetImage::PixelType m_LabelUnLocked1 = 3;
  mitk::LabelSetImage::PixelType m_LabelUnLocked2 = 4;

public:
  void setUp() override
  {
    m_SourceImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_source.nrrd"));
  }

  void tearDown() override
  {
    m_SourceImage = nullptr;
  }

  void TestTransfer_defaults()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks.nrrd"));
    auto refLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedExteriorImage);

    CPPUNIT_ASSERT_MESSAGE("Transfer with default settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with default settings + exterior lock failed",
      mitk::Equal(*(destinationLockedExteriorImage.GetPointer()), *(refLockedExteriorImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Merge_RegardLocks()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_regardLocks.nrrd"));
    auto refLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_regardLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedExteriorImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + regardLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + regardLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedExteriorImage.GetPointer()), *(refLockedExteriorImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Merge_IgnoreLocks()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_ignoreLocks.nrrd"));
    auto refLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_ignoreLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedExteriorImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + ignoreLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + ignoreLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedExteriorImage.GetPointer()), *(refLockedExteriorImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Replace_RegardLocks()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks.nrrd"));
    auto refLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedExteriorImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + regardLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + regardLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedExteriorImage.GetPointer()), *(refLockedExteriorImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Replace_IgnoreLocks()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_ignoreLocks.nrrd"));
    auto refLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_ignoreLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedExteriorImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + ignoreLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + ignoreLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedExteriorImage.GetPointer()), *(refLockedExteriorImage.GetPointer()), mitk::eps, false));
  }


  void TestTransfer_multipleLabels()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_multipleLabels.nrrd"));
    auto refLockedExteriorImage = mitk::IOUtil::Load<mitk::LabelSetImage>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_multipleLabels_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1}, {3,1}, {2,4}, {4,2} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedExteriorImage, { {1,1}, {3,1}, {2,4}, {4,2} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer multiple labels (1->1, 3->1, 2->4, 4->2) with replace + ignoreLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer multiple labels (1->1, 3->1, 2->4, 4->2) with replace + ignoreLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedExteriorImage.GetPointer()), *(refLockedExteriorImage.GetPointer()), mitk::eps, false));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkTransferLabel)
