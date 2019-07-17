Name:		ricochet
Version:	1.1.0
Release:	1%{?dist}
Summary:	Anonymous peer-to-peer instant messaging

License:	BSD
URL:		https://ricochet.im/
Source0:	https://ricochet.im/releases/%{version}/ricochet-%{version}-src.tar.bz2

BuildRequires:	openssl-devel
BuildRequires:	protobuf-compiler
BuildRequires:	protobuf-devel
BuildRequires:	qt5-qtbase-devel
BuildRequires:	qt5-qtbase-gui
BuildRequires:	qt5-qtdeclarative-devel
BuildRequires:	qt5-qtmultimedia-devel
BuildRequires:	qt5-qtquickcontrols
BuildRequires:	qt5-qttools-devel
Requires:	openssl-libs
Requires:	protobuf
Requires:	qt5-qtbase
Requires:	qt5-qtbase-gui
Requires:	qt5-qtdeclarative
Requires:	qt5-qtquickcontrols
Requires:	qt5-qtmultimedia
Requires:	tor

%description
Ricochet is an experiment with a different kind of instant messaging that doesn't trust anyone with your identity, your contact list, or your communications.
 * You can chat without exposing your identity (or IP address) to anyone
 * Nobody can discover who your contacts are or when you talk (metadata-free!)
 * There are no servers to compromise or operators to intimidate for your information
 * It's cross-platform and easy for non-technical users


%prep
%setup -q


%build
qmake-qt5 DEFINES+=RICOCHET_NO_PORTABLE CONFIG+=release
make -f Makefile.Release %{?_smp_mflags}


%install
make -f Makefile.Release install INSTALL_ROOT=%{buildroot}
install -m 0644 -D -p LICENSE %{buildroot}/%{_docdir}/%{name}/LICENSE
install -m 0644 -D -p AUTHORS.md %{buildroot}/%{_docdir}/%{name}/AUTHORS.md
install -m 0644 -D -p README.md %{buildroot}/%{_docdir}/%{name}/README.md


%files
/usr/bin/ricochet
/usr/share/applications/ricochet.desktop
/usr/share/icons/hicolor/48x48/apps/ricochet_refresh.png
/usr/share/icons/hicolor/scalable/apps/ricochet_refresh.svg
%docdir %{_docdir}/%{name}
%doc %{_docdir}/%{name}/LICENSE
%doc %{_docdir}/%{name}/AUTHORS.md
%doc %{_docdir}/%{name}/README.md



%changelog
* Mon Jul 27 2015 Peter Ludikovsky <peter@ludikovsky.name> 1.1.0-1
- Initial RPM Package
