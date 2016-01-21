# -*- coding: utf-8 -*-
"""
Created on Fri Aug 14 11:09:18 2015

@author: wirkert
"""


import os
import Image
import ImageEnhance
import pickle
import logging
import datetime

import numpy as np
import pandas as pd
import luigi
import matplotlib.pyplot as plt
import SimpleITK as sitk
from sklearn.ensemble.forest import RandomForestRegressor
from scipy.ndimage.filters import gaussian_filter

import tasks_analyze as at
import tasks_mc
import scriptpaths as sp
from msi.msi import Msi
from msi.io.pngreader import PngReader
from msi.io.nrrdreader import NrrdReader
from msi.io.nrrdwriter import NrrdWriter
import msi.msimanipulations as msimani
import msi.imgmani as imgmani
import msi.normalize as norm
from regression.estimation import estimate_image
from regression.linear import LinearSaO2Unmixing
from regression.preprocessing import preprocess, preprocess2
from msi.io.tiffringreader import TiffRingReader
from sklearn.linear_model.base import LinearRegression


sp.ROOT_FOLDER = "/media/wirkert/data/Data/" + \
            "2015_11_12_IPCAI_in_vivo"
sp.DATA_FOLDER = "small_bowel_images"
sp.FINALS_FOLDER = "small_bowel_images_jet"
sp.FLAT_FOLDER = "flatfields_liver"
sp.DARK_FOLDER = "/media/wirkert/data/Data/" + \
                 "2016_01_19_Flatfield_and_Dark_Laparoscope/Dark"

AVERAGE_FOLDER = "average_intensity"
AVERAGE_FINALS_FOLDER = "finals"

# sp.RECORDED_WAVELENGTHS = np.arange(470, 680, 10) * 10 ** -9

new_order = [0, 1, 2, 3, 4, 5, 6, 7]


def get_image_files_from_folder(folder):
    # small helper function to get all the image files in a folder
    image_files = [f for f in os.listdir(folder) if
                   os.path.isfile(os.path.join(folder, f))]
    image_files.sort()
    image_files = [f for f in image_files if f.endswith(".tiff")]
    return image_files


def resort_image_wavelengths(collapsed_image):
    return collapsed_image[:, new_order]


def resort_wavelengths(msi):
    """Neil organized his _wavelengths differently.
    This function fixes this."""
    collapsed_image = imgmani.collapse_image(msi.get_image())
    collapsed_image = resort_image_wavelengths(collapsed_image)
    msi.set_image(np.reshape(collapsed_image, msi.get_image().shape))

# rebind this method since in this recoding the _wavelengths got messed up
sp.resort_wavelengths = resort_wavelengths

sp.bands_to_sortout = np.array([])  # [0, 1, 2, 20, 19, 18, 17, 16])


class IPCAICreateOxyImageTask(luigi.Task):
    image_name = luigi.Parameter()
    df_prefix = luigi.Parameter()

    def requires(self):
        return IPCAITrainRegressor(df_prefix=self.df_prefix), \
               Flatfield(), \
               SingleMultispectralImage(image=self.image_name)

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              self.image_name + "_" +
                                              self.df_prefix +
                                              "_summary.png.png"))

    def run(self):
        nrrd_reader = NrrdReader()
        tiff_ring_reader = TiffRingReader()
        # read the flatfield
        flat = nrrd_reader.read(self.input()[1].path)
        # read the msi
        nr_filters = len(sp.RECORDED_WAVELENGTHS)
        msi, segmentation = tiff_ring_reader.read(self.input[2].path,
                                                  nr_filters)
        segmentation = np.logical_and(segmentation,
                                      (np.max(msi.get_image(),
                                              axis=-1) < 1000.))

        # read the regressor
        e_file = open(self.input()[0].path, 'r')
        e = pickle.load(e_file)

        # correct image setup
        filter_nr = int(self.image_name[-6:-5])
        original_order = np.arange(nr_filters)
        new_image_order = np.concatenate((
                                original_order[nr_filters - filter_nr:],
                                original_order[:nr_filters - filter_nr]))
        # resort msi to restore original order
        msimani.get_bands(msi, new_image_order)
        # correct by flatfield
        msimani.flatfield_correction(msi, flat)

        # create artificial rgb
        rgb_image = msi.get_image()[:, :, [2, 3, 1]]
        rgb_image /= np.max(rgb_image)
        rgb_image *= 255.

        # preprocess the image
        # sortout unwanted bands
        print "1"
        msi.set_image(imgmani.sortout_bands(msi.get_image(),
                                            sp.bands_to_sortout))
        # zero values would lead to infinity logarithm, thus clip.
        msi.set_image(np.clip(msi.get_image(), 0.00001, 2. ** 64))
        # normalize to get rid of lighting intensity
        norm.standard_normalizer.normalize(msi)
        # transform to absorption
        msi.set_image(-np.log(msi.get_image()))
        # normalize by l2 for stability
        norm.standard_normalizer.normalize(msi, "l2")
        print "2"
        # estimate
        sitk_image = estimate_image(msi, e)
        image = sitk.GetArrayFromImage(sitk_image)

        plt.figure()
        print "3"
        rgb_image = rgb_image.astype(np.uint8)
        im = Image.fromarray(rgb_image, 'RGB')
        enh_brightness = ImageEnhance.Brightness(im)
        im = enh_brightness.enhance(5.)
        plot_image = np.array(im)
        top_left_axis = plt.gca()
        top_left_axis.imshow(plot_image, interpolation='nearest')
        top_left_axis.xaxis.set_visible(False)
        top_left_axis.yaxis.set_visible(False)

        plt.set_cmap("jet")
        print "4"
        # plot parametric maps
        oxy_image = image[:, :]
        segmentation[0, 0] = 1
        segmentation[0, 1] = 1
        oxy_image = np.ma.masked_array(image[:, :], (segmentation < 1))
        oxy_image[np.isnan(oxy_image)] = 0.
        oxy_image[np.isinf(oxy_image)] = 0.
        oxy_mean = np.mean(oxy_image)
        oxy_image[0, 0] = 0.0
        oxy_image[0, 1] = 1.
#        oxy_image = np.clip(oxy_image, 0.2, 0.8)
#         oxy_image[oxy_image > 0.8] = 0.8
#         oxy_image[oxy_image < 0.2] = 0.2

        at.plot_axis(plt.gca(), oxy_image[:, :],
                  "oxygenation [%]. Mean " +
                  "{0:.1f}".format((oxy_mean * 100.)) + "%")

        plt.savefig(self.output().path, dpi=500, bbox_inches='tight')
#         plt.figure()
#         bvf_image = np.ma.masked_array(image[:, :, 0], (segmentation < 1))
#         # bvf_image = image[:, :, 0]
#         bvf_image[np.isnan(bvf_image)] = 0.
#         bvf_image[np.isinf(bvf_image)] = 0.
#         bvf_image[bvf_image > 1.] = 1.
#         bvf_image[bvf_image < 0.] = 0.
#         bvf_mean = np.mean(bvf_image)
#         bvf_image[0, 0] = 0.
#         bvf_image[1, 1] = 0.1
#         at.plot_axis(plt.gca(), bvf_image[:, :] * 100,
#                   "blood volume fraction [%]. Mean " + "{0:.1f}".format((bvf_mean * 100.)) + "%")
#
#         plt.savefig(self.output().path + "_bvf.png", dpi=500, bbox_inches='tight')
        print "4.5"

#         fd = open(os.path.join(sp.ROOT_FOLDER, sp.RESULTS_FOLDER,
#                                    sp.FINALS_FOLDER,
#                                    'results_summary.csv'), 'a')
#         fd.write(self.image_name + ", " + str(int(oxy_mean * 100)) +
#                  ", " + "{0:.1f}".format((bvf_mean * 100.)) + "\n")
#         fd.close()
        print "5"
        plt.close("all")


class IPCAITrainRegressor(luigi.Task):
    df_prefix = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                              sp.FINALS_FOLDER,
                                              "reg_" +
                                              self.df_prefix))

    def requires(self):
        return tasks_mc.CameraBatch(self.df_prefix)

    def run(self):
        # extract data from the batch
        f = open(self.input().path, "r")
        batch_train = pickle.load(f)
        # TODO: fix hack by sorting always to ascending _wavelengths
        batch_train.reflectances = resort_image_wavelengths(
                                                    batch_train.reflectances)
        batch_train._wavelengths = batch_train._wavelengths[new_order]
        X, y = preprocess2(batch_train,
                          w_percent=0.1, bands_to_sortout=sp.bands_to_sortout)

        # train regressor
        reg = RandomForestRegressor(10, min_samples_leaf=10, max_depth=9,
                                   n_jobs=-1)
        # reg = KNeighborsRegressor(algorithm="auto")
        # reg = LinearRegression()
        # reg = sklearn.svm.SVR(kernel="rbf", degree=3, C=100., gamma=10.)
        # reg = LinearSaO2Unmixing()
        reg.fit(X, y[:, 1])
        # reg = LinearSaO2Unmixing()
        # save regressor
        f = self.output().open('w')
        pickle.dump(reg, f)
        f.close()


class SingleMultispectralImage(luigi.Task):

    image = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER, sp.DATA_FOLDER, \
                                              self.image))


class Flatfield(luigi.Task):
    flatfield_folder = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                "flatfield" +
                                ".nrrd"))

    def run(self):
        tiff_ring_reader = TiffRingReader()
        nr_filters = len(sp.RECORDED_WAVELENGTHS)

        # analyze all the first image files
        image_files = get_image_files_from_folder(self.flatfield_folder)
        image_files = filter(lambda image_name: "F0" in image_name, image_files)

        # helper function to take maximum of two images
        def maximum_of_two_images(image_name_1, image_name_2):
            image_1 = tiff_ring_reader.read(image_name_1,
                                            nr_filters)[0].get_image()
            image_2 = tiff_ring_reader.read(image_name_2,
                                            nr_filters)[0].get_image()
            return np.maximum(image_1, image_2)

        # now reduce to maximum of all the single images
        flat_maximum = reduce(lambda x, y: maximum_of_two_images(x, y),
                              image_files)
        msi = Msi(image=flat_maximum)
        msi.set_wavelengths(sp.RECORDED_WAVELENGTHS)

        # write flatfield as nrrd
        writer = NrrdWriter(msi)
        writer.write(self.output().path)


class Dark(luigi.Task):
    dark_folder = luigi.Parameter()

    def output(self):
        return luigi.LocalTarget(os.path.join(sp.ROOT_FOLDER,
                                              sp.RESULTS_FOLDER,
                                "dark" +
                                ".nrrd"))

    def run(self):
        tiff_ring_reader = TiffRingReader()
        nr_filters = len(sp.RECORDED_WAVELENGTHS)

        # analyze all the first image files
        image_files = get_image_files_from_folder(self.dark_folder)
        image_files = filter(lambda image_name: "F0" in image_name, image_files)

        # returns the mean dark image vector of all inputted dark image
        # overly complicated TODO SW: make this simple code readable.
        dark_means = map(lambda image_name:
                            msimani.calculate_mean_spectrum(
                                tiff_ring_reader.read(os.path.join(self.dark_folder, image_name),
                                                      nr_filters)[0]),
                         image_files)
        dark_means_sum = reduce(lambda x, y: x+y.get_image(), dark_means, 0)
        final_dark_mean = dark_means_sum / len(dark_means)

        msi = Msi(image=final_dark_mean)
        msi.set_wavelengths(sp.RECORDED_WAVELENGTHS)

        # write flatfield as nrrd
        writer = NrrdWriter(msi)
        writer.write(self.output().path)


if __name__ == '__main__':

    # root folder there the data lies
    logging.basicConfig(filename='small_bowel' + str(datetime.datetime.now()) +
                         '.log', level=logging.INFO)
    luigi.interface.setup_interface_logging()
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)

    # run over all subfolders (non-recursively)
    # to collect the data and generate the results
    image_file_folder = os.path.join(sp.ROOT_FOLDER, sp.DATA_FOLDER)
    onlyfiles = get_image_files_from_folder(image_file_folder)

    for f in onlyfiles:
        main_task = IPCAICreateOxyImageTask(image_name=f,
                                    df_prefix="ipcai_colon_muscle_train")
        w.add(main_task)

    w.run()

