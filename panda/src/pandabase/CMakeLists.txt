include_directories(../../../dtool/src/dtoolbase)

set(P3PANDABASE_HEADERS
    pandabase.h pandasymbols.h
)

set(P3PANDABASE_SOURCES
    pandabase.cxx
)

add_library(p3pandabase STATIC
	${P3PANDABASE_HEADERS}
	${P3PANDABASE_SOURCES}
)
