
#QT += gui
#QT += widgets

!win32-msvc* {
    QMAKE_CXXFLAGS += -Wextra -pedantic
}

INCLUDEPATH += \
    $$PWD/qssh/src/libs/ \
    $$PWD/qssh/src/libs/qssh/

SOURCES += \
    $$PWD/qssh/src/libs/qssh/opensshkeyfilereader.cpp \
    $$PWD/qssh/src/libs/qssh/sftpchannel.cpp \
    $$PWD/qssh/src/libs/qssh/sftpdefs.cpp \
    $$PWD/qssh/src/libs/qssh/sftpfilesystemmodel.cpp \
    $$PWD/qssh/src/libs/qssh/sshincomingpacket.cpp \
    $$PWD/qssh/src/libs/qssh/sftpoperation.cpp \
    $$PWD/qssh/src/libs/qssh/sshoutgoingpacket.cpp \
    $$PWD/qssh/src/libs/qssh/sftppacket.cpp \
    $$PWD/qssh/src/libs/qssh/sshagent.cpp \
    $$PWD/qssh/src/libs/qssh/sshcapabilities.cpp \
    $$PWD/qssh/src/libs/qssh/sshchannel.cpp \
    $$PWD/qssh/src/libs/qssh/sshchannelmanager.cpp \
    $$PWD/qssh/src/libs/qssh/sshconnection.cpp \
    $$PWD/qssh/src/libs/qssh/sshconnectionmanager.cpp \
    $$PWD/qssh/src/libs/qssh/sshcryptofacility.cpp \
    $$PWD/qssh/src/libs/qssh/sshdirecttcpiptunnel.cpp \
    $$PWD/qssh/src/libs/qssh/sshforwardedtcpiptunnel.cpp \
    $$PWD/qssh/src/libs/qssh/sshhostkeydatabase.cpp \
    $$PWD/qssh/src/libs/qssh/sshkeyexchange.cpp \
    $$PWD/qssh/src/libs/qssh/sshkeygenerator.cpp \
    $$PWD/qssh/src/libs/qssh/sshkeypasswordretriever.cpp \
    $$PWD/qssh/src/libs/qssh/sshlogging.cpp \
    $$PWD/qssh/src/libs/qssh/sshpacket.cpp \
    $$PWD/qssh/src/libs/qssh/sshpacketparser.cpp \
    $$PWD/qssh/src/libs/qssh/sshremoteprocess.cpp \
    $$PWD/qssh/src/libs/qssh/sshremoteprocessrunner.cpp \
    $$PWD/qssh/src/libs/qssh/sshsendfacility.cpp \
    $$PWD/qssh/src/libs/qssh/sshtcpipforwardserver.cpp \
    $$PWD/qssh/src/libs/qssh/sshtcpiptunnel.cpp \
    $$PWD/qssh/src/libs/qssh/sshx11channel.cpp \
    $$PWD/qssh/src/libs/qssh/sshx11inforetriever.cpp


HEADERS += \
    $$PWD/qssh/src/libs/qssh/opensshkeyfilereader_p.h \
    $$PWD/qssh/src/libs/qssh/sshincomingpacket_p.h \
    $$PWD/qssh/src/libs/qssh/sshoutgoingpacket_p.h \
    $$PWD/qssh/src/libs/qssh/sftpchannel.h \
    $$PWD/qssh/src/libs/qssh/sftpchannel_p.h \
    $$PWD/qssh/src/libs/qssh/sftpdefs.h \
    $$PWD/qssh/src/libs/qssh/sftpfilesystemmodel.h \
    $$PWD/qssh/src/libs/qssh/sftpoperation_p.h \
    $$PWD/qssh/src/libs/qssh/sftppacket_p.h \
    $$PWD/qssh/src/libs/qssh/sshagent_p.h \
    $$PWD/qssh/src/libs/qssh/sshbotanconversions_p.h \
    $$PWD/qssh/src/libs/qssh/sshcapabilities_p.h \
    $$PWD/qssh/src/libs/qssh/sshchannelmanager_p.h \
    $$PWD/qssh/src/libs/qssh/sshchannel_p.h \
    $$PWD/qssh/src/libs/qssh/sshconnection.h \
    $$PWD/qssh/src/libs/qssh/sshconnectionmanager.h \
    $$PWD/qssh/src/libs/qssh/sshconnection_p.h \
    $$PWD/qssh/src/libs/qssh/sshcryptofacility_p.h \
    $$PWD/qssh/src/libs/qssh/sshdirecttcpiptunnel.h \
    $$PWD/qssh/src/libs/qssh/sshdirecttcpiptunnel_p.h \
    $$PWD/qssh/src/libs/qssh/ssherrors.h \
    $$PWD/qssh/src/libs/qssh/sshexception_p.h \
    $$PWD/qssh/src/libs/qssh/sshforwardedtcpiptunnel.h \
    $$PWD/qssh/src/libs/qssh/sshforwardedtcpiptunnel_p.h \
    $$PWD/qssh/src/libs/qssh/ssh_global.h \
    $$PWD/qssh/src/libs/qssh/sshhostkeydatabase.h \
    $$PWD/qssh/src/libs/qssh/sshkeyexchange_p.h \
    $$PWD/qssh/src/libs/qssh/sshkeygenerator.h \
    $$PWD/qssh/src/libs/qssh/sshkeypasswordretriever_p.h \
    $$PWD/qssh/src/libs/qssh/sshlogging_p.h \
    $$PWD/qssh/src/libs/qssh/sshpacketparser_p.h \
    $$PWD/qssh/src/libs/qssh/sshpacket_p.h \
    $$PWD/qssh/src/libs/qssh/sshpseudoterminal.h \
    $$PWD/qssh/src/libs/qssh/sshremoteprocess.h \
    $$PWD/qssh/src/libs/qssh/sshremoteprocess_p.h \
    $$PWD/qssh/src/libs/qssh/sshremoteprocessrunner.h \
    $$PWD/qssh/src/libs/qssh/sshsendfacility_p.h \
    $$PWD/qssh/src/libs/qssh/sshtcpipforwardserver.h \
    $$PWD/qssh/src/libs/qssh/sshtcpipforwardserver_p.h \
    $$PWD/qssh/src/libs/qssh/sshtcpiptunnel_p.h \
    $$PWD/qssh/src/libs/qssh/sshx11channel_p.h \
    $$PWD/qssh/src/libs/qssh/sshx11displayinfo_p.h \
    $$PWD/qssh/src/libs/qssh/sshx11inforetriever_p.h

RESOURCES += \
    $$PWD/qssh/src/libs/qssh/qssh.qrc

# botan
isEmpty(BOTANPATH): BOTANPATH = $${PWD}/qssh/botan

unix {
    INCLUDEPATH += $$BOTANPATH \
        $$BOTANPATH/build/include
    # use static lib
    LIBS += $$OUT_PWD/../lib/qssh/lib/libQSsh.a
    LIBS += -Wl,--whole-archive
    LIBS += $$BOTANPATH/libbotan-2.a
    LIBS += -Wl,--no-whole-archive
}
win32:{
    LIBS += -L$$OUT_PWD/../lib/qssh/lib/ \
        $$OUT_PWD/../lib/qssh/lib/QSsh.lib
    INCLUDEPATH += $$BOTANPATH \
        $$BOTANPATH/build/include
    LIBS += $$BOTANPATH/botan.lib
    LIBS += -lUser32
}
