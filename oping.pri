
unix :{
    CONFIG += link_pkgconfig
    PKGCONFIG += liboping
    LIBS += $$system(pkg-config --libs liboping)
    #INCLUDEPATH+= $$system(pkg-config)

    #LIBS += -loping
    #INCLUDEPATH += /usr/include/oping # Path to liboping headers
}
