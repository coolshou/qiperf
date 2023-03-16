#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sys
import os
try:
    from PyQt5.QtCore import (Qt, pyqtSignal, pyqtSlot)
    from PyQt5.QtWidgets import (QDialog, QTableWidgetItem, QAbstractButton,
                                 QDialogButtonBox)
    from PyQt5.QtGui import QIcon
    from PyQt5.uic import loadUi
except ImportError as err:
    raise SystemExit("pip install PyQt5\n %s" % err)
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

# from xmlnodedlg import XmlNodeDlg
from throughputCol import TpCol


class ThroughputEdit(QDialog):
    '''helper class to edit throughput setting (iperf or chariot)'''
    closeMe = pyqtSignal()  # when we want to close, Must Have signal
    rejected = pyqtSignal()

    # def __init__(self, sType, sName, sDesc, sData, parent=None):
    def __init__(self, irow, treeview, parent=None):
        # item: ProjectItem
        super(ThroughputEdit, self).__init__(parent)
        if getattr(sys, 'frozen', False):
            # we are running in a |PyInstaller| bundle
            basedir = sys._MEIPASS
        else:
            # we are running in a normal Python environment
            basedir = os.path.dirname(__file__)
        loadUi(os.path.join(basedir, 'throughputEdit.ui'), self)
        self.buttonBox.accepted.connect(self.on_accepted)
        self.buttonBox.rejected.connect(self.on_rejected)
        # self.buttonBox.clicked.connect(self.on_clicked)
        self.cb_type.currentIndexChanged.connect(self.on_type_changed)
        self.tw_iperf.itemSelectionChanged.connect(self.on_iperf_selectionChanged)
        #
        self.irow = irow
        self.tv = treeview
        self.sType = self.tv.item(irow, TpCol.get("type")).text()
        self.sName = self.tv.item(irow, TpCol.get("name")).text()
        self.sDesc = self.tv.item(irow, TpCol.get("desc")).text()
        self.sData = self.tv.item(irow, TpCol.get("data")).text()

        self.update_ui()

    @pyqtSlot()
    def on_accepted(self):
        '''save UI values back to xmlnode")'''
        print("TODO: agentDlg save UI values")
        items = self.tw_iperf.selectedItems()
        if len(items) > 0:
            self._save_iperf_data(items[0])
        self.closeMe.emit()
        self.close()

    @pyqtSlot()
    def on_rejected(self):
        # self.closeMe.emit()
        self.rejected.emit()
        self.closeMe.emit()
        self.close()

    @pyqtSlot(QAbstractButton)
    def on_clicked(self, button):
        print("on_clicked button: (%s)" % (type(button)))
        if button == QDialogButtonBox.button(QDialogButtonBox.Save):
            print("save data")
        if button == QDialogButtonBox.button(QDialogButtonBox.Cancel):
            print("Cancel")
        if button == QDialogButtonBox.button(QDialogButtonBox.Close):
            print("Close")

    @pyqtSlot(int)
    def on_type_changed(self, idx):
        print("on_type_changed: %s" % idx)
        if idx == 1:
            # iperf
            self.tw_type.setTabEnabled(0, True)
            self.tw_type.setTabEnabled(1, False)
        elif idx == 2:
            self.tw_type.setTabEnabled(0, False)
            self.tw_type.setTabEnabled(1, True)
        else:
            self.tw_type.setTabEnabled(0, True)
            self.tw_type.setTabEnabled(1, True)

    def update_ui(self):
        idx = self.cb_type.findText(self.sType)
        if idx >= 0:
            self.cb_type.setCurrentIndex(idx)

        self.le_name.setText(self.sName)
        self.le_desc.setText(self.sDesc)
        # parser data
        if self.sType == "iperf":
            self._load_iperf_datarow()
        else:
            self._load_chariot_data()

    @pyqtSlot()
    def on_iperf_selectionChanged(self):
        items = self.tw_iperf.selectedItems()
        print("on_iperf_selectionChanged: %s" % items)
        if len(items) > 0:
            self._load_iperf_data(items[0])

    def _save_iperf_data(self, item):
        '''save ui data to item'''
        sdata = item.text()
        ds = eval(sdata)  # dict
        ds.update({"mIPserver": self.le_mIPserver.text(),
                   "mIPclient": self.le_mIPclient.text(),
                   "server": self.le_server.text(),
                   "client": self.le_client.text(),
                   "protocal": self.cb_protocal.currentIndex(),
                   "duration": self.sb_duration.value(),
                   "parallel": self.sb_parallel.value(),
                   "reverse": self.chk_reverse.isChecked(),
                   "bitrate": self.sb_bitrate.value(),
                   "unit_bitrate": self.cb_unit_bitrate.currentText(),
                   "windowsize": self.sb_windowsize.value(),
                   "unit_windowsize": self.cb_unit_windowsize.currentText(),
                   "dscp": self.sb_dscp.value(),
                   "maximum_segment_size": self.sb_mss.value(),
                   "omit": self.sb_omit.value(),
                   "fmtreport": self.cb_fmtreport.currentText(),
                   "version": int(self.cb_version.currentText())})

        item.setText(str(ds))

        # type, name, desc, data
        self.tv.item(self.irow, TpCol.get("type")).setText(self.cb_type.currentText())
        self.sName = self.tv.item(self.irow, TpCol.get("name")).setText(self.le_name.text())
        self.sDesc = self.tv.item(self.irow, TpCol.get("desc")).setText(self.le_desc.text())
        data = []
        for idx in range(self.tw_iperf.rowCount()):
            item = self.tw_iperf.item(idx, 0)
            if item:
                data.append(item.text())
        print("data: %s" % (data,))
        self.tv.item(self.irow, TpCol.get("data")).setText(str(data))

    def _load_iperf_data(self, item):
        '''show item's  data to ui'''
        sdata = item.text()
        ds = eval(sdata)
        print("_load_iperf_data: %s" % (ds,))
        # {'mIPserver': '192.168.110.13', 'mIPclient': '192.168.110.111',
        # 'server': '192.168.0.13', 'client': '192.168.0.111',
        # 'protocal': 1, 'duration': 900, 'parallel': 1,
        # 'reverse': 0,
        # 'bitrate': 50, 'unit_bitrate': 'M', 'windowsize': 0, 'unit_windowsize': 'K',
        # 'omit': 2, 'fmtreport': 'm', 'version': 3}
        self.le_mIPserver.setText(ds.get("mIPserver", ""))
        self.le_mIPclient.setText(ds.get("mIPclient", ""))
        self.le_server.setText(ds.get("server", ""))
        self.le_client.setText(ds.get("client", ""))
        self.cb_protocal.setCurrentIndex(ds.get("protocal", 0))
        self.sb_duration.setValue(ds.get("duration", 10))
        self.sb_parallel.setValue(ds.get("parallel", 1))
        self.chk_reverse.setChecked(ds.get("reverse", False))
        self.sb_bitrate.setValue(ds.get("bitrate", 0))
        idx = self.cb_unit_bitrate.findText(ds.get("unit_bitrate", "M"))
        if idx >= 0:
            self.cb_unit_bitrate.setCurrentIndex(idx)
        self.sb_windowsize.setValue(ds.get("windowsize", 0))
        idx = self.cb_unit_windowsize.findText(ds.get("unit_windowsize", "K"))
        if idx >= 0:
            self.cb_unit_windowsize.setCurrentIndex(idx)
        self.sb_dscp.setValue(ds.get("dscp", -1))
        self.sb_mss.setValue(ds.get("maximum_segment_size", 0))
        self.sb_omit.setValue(ds.get("omit", 2))
        idx = self.cb_fmtreport.findText(ds.get("fmtreport", "m"))
        if idx >= 0:
            self.cb_fmtreport.setCurrentIndex(idx)
        idx = self.cb_version.findText(str(ds.get("version", 3)))
        if idx >= 0:
            self.cb_version.setCurrentIndex(idx)

    def _load_iperf_datarow(self):
        print("self.sData(%s): %s" % (type(self.sData), self.sData))
        ds = eval(self.sData)
        print("ds: %s" % (ds,))
        irows = len(ds)
        self.tw_iperf.setRowCount(irows)
        for idx in range(len(ds)):
            # irow = self.tw_iperf.rowCount()
            itm = QTableWidgetItem("%s" % (ds[idx],))
            # itm.setText()
            self.tw_iperf.setItem(idx, 0, itm)
        irow = self.tw_iperf.rowCount()
        print("irow: %s" % irow)
        self.tw_iperf.setCurrentCell(0, 0)

    def save_iperf_data(self):
        print("save_iperf_data")

    def _load_chariot_data(self):
        print("TODO:_load_chariot_data")

