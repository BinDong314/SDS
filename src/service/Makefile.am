AUTOMAKE_OPTIONS=gnu
bin_PROGRAMS=$(top_builddir)/bin/vpic_energy_sorter $(top_builddir)/bin/openmsi_transposer  $(top_builddir)/bin/chunk_sorter  $(top_builddir)/bin/fq_index_builder $(top_builddir)/bin/h5group-sorter $(top_builddir)/bin/sfc $(top_builddir)/bin/concatenation

if HAVE_HDF5_VOL
__top_builddir__bin_vpic_energy_sorter_SOURCES=vpic-energy-sorter.c sds-vol-external-native.c
else
__top_builddir__bin_vpic_energy_sorter_SOURCES=vpic-energy-sorter.c
endif
__top_builddir__bin_vpic_energy_sorter_CFLAGS=@SDS_SERVER_CFLAGS@
__top_builddir__bin_vpic_energy_sorter_CPPFLAGS=@SDS_SERVER_CPPFLAGS@
__top_builddir__bin_vpic_energy_sorter_LDFLAGS=@SDS_SERVER_LIBS@

if HAVE_HDF5_VOL
__top_builddir__bin_openmsi_transposer_SOURCES=openmsi-transposer.c sds-vol-external-native.c
else
__top_builddir__bin_openmsi_transposer_SOURCES=openmsi-transposer.c 
endif
__top_builddir__bin_openmsi_transposer_CFLAGS=@SDS_SERVER_CFLAGS@
__top_builddir__bin_openmsi_transposer_CPPFLAGS=@SDS_SERVER_CPPFLAGS@
__top_builddir__bin_openmsi_transposer_LDFLAGS=@SDS_SERVER_LIBS@


__top_builddir__bin_h5group_sorter_SOURCES=h5group-sorter.c
__top_builddir__bin_h5group_sorter_CFLAGS=@SDS_SERVER_CFLAGS@
__top_builddir__bin_h5group_sorter_CPPFLAGS=@SDS_SERVER_CPPFLAGS@
__top_builddir__bin_h5group_sorter_LDFLAGS=@SDS_SERVER_LIBS@

__top_builddir__bin_chunk_sorter_SOURCES=chunk-sort.c
__top_builddir__bin_chunk_sorter_CFLAGS=@SDS_SERVER_CFLAGS@
__top_builddir__bin_chunk_sorter_CPPFLAGS=@SDS_SERVER_CPPFLAGS@
__top_builddir__bin_chunk_sorter_LDFLAGS=@SDS_SERVER_LIBS@

__top_builddir__bin_fq_index_builder_SOURCES=fq-index-builder.cc 
__top_builddir__bin_fq_index_builder_CFLAGS=@SDS_SERVER_CFLAGS@
__top_builddir__bin_fq_index_builder_CPPFLAGS=@SDS_SERVER_CPPFLAGS@
__top_builddir__bin_fq_index_builder_LDFLAGS=@SDS_SERVER_LIBS@

__top_builddir__bin_sfc_SOURCES=sfc.c
__top_builddir__bin_sfc_CFLAGS=@SDS_SERVER_CFLAGS@
__top_builddir__bin_sfc_CPPFLAGS=@SDS_SERVER_CPPFLAGS@
__top_builddir__bin_sfc_LDFLAGS=@SDS_SERVER_LIBS@

__top_builddir__bin_concatenation_SOURCES=concatenation.c
__top_builddir__bin_concatenation_CFLAGS=@SDS_SERVER_CFLAGS@
__top_builddir__bin_concatenation_CPPFLAGS=@SDS_SERVER_CPPFLAGS@
__top_builddir__bin_concatenation_LDFLAGS=@SDS_SERVER_LIBS@

