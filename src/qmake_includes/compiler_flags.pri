# link-time code generation/optimization
CONFIG += ltcg

unix {
    # get us onto the latest c++
    QMAKE_CXXFLAGS += --std=c++2a
}