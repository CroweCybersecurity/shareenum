SAMBAROOT=/usr/local/samba/lib
BINFILE=../src/shareenum
DSTDIR=/usr/local/shareenum

install:
	cp $(BINFILE) $(DSTDIR)/bin/

deb: 
	fpm -s dir -t deb -n "shareenum" -v 2.0.rc1 --before-remove beforeremove.sh --after-install afterinstall.sh --url https://github.com/emperorcow/shareenum --description "Tool for gathering permissions from MS shares" $(DSTDIR)/

libprep: 
	cp $(SAMBAROOT)/libsmbclient.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libsamba-util.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libreplace.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libmsrpc3.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/liblibcli_lsa3.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/liblibsmb.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libsamba-security.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/liberrors.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libsmbconf.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libndr.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libgse.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libndr-standard.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libdcerpc-samba.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libcli_smb_common.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libutil_cmdline.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libsmbregistry.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libsecrets3.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libtevent.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libndr-samba.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libsamba-credentials.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libsamba-sockets.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libdcerpc-binding.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libcliauth.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libgensec.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libtevent-util.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libsamba3-util.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libwbclient.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libcli_cldap.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libkrb5samba.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libcom_err-samba4.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libasn1util.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libsamba-hostconfig.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libndr-nbt.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libsmb_transport.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libCHARSET3.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libinterfaces.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libccan.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libdbwrap.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libutil_tdb.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libutil_reg.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libsmbd_shim.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libutil_setid.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libtdb-wrap.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libserver-role.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libkrb5-samba4.so.26 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libaddns.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libgssapi-samba4.so.2 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libauthkrb5.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libcli-nbt.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libldb.so.1 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libsamdb-common.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libldbsamba.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libsamba-modules.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libsamdb.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libwinbind-client.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libcli-ldap-common.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libasn1-samba4.so.8 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libutil_ntdb.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libntdb.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libheimbase-samba4.so.1 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libhx509-samba4.so.5 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libhcrypto-samba4.so.5 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libroken-samba4.so.19 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libwind-samba4.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/libndr-krb5pac.so.0 $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libauth_sam_reply.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libflag_mapping.so $(DSTDIR)/lib/
	cp $(SAMBAROOT)/private/libtdb.so.1 $(DSTDIR)/lib/
