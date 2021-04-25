%global repo dde-calendar

Name:           deepin-calendar
Version:        5.7.0.19
Release:        1%{?dist}
Summary:        Calendar for Deepin Desktop Environment
License:        GPLv3+
URL:            https://github.com/linuxdeepin/dde-calendar
Source0:        %{url}/archive/%{version}/%{repo}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  deepin-gettext-tools
BuildRequires:  desktop-file-utils
BuildRequires:  qt5-linguist
BuildRequires:  pkgconfig(dtkwidget) >= 2.0
BuildRequires:  pkgconfig(dframeworkdbus)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5X11Extras)
Requires:       hicolor-icon-theme

%description
Calendar for Deepin Desktop Environment.

%prep
%autosetup -p1 -n %{repo}-%{version}

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
%cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo
%cmake_build

%install
%cmake_install
install -m 644 -D assets/resources/icon/dde-calendar.svg ${RPM_BUILD_ROOT}%{_datadir}/icons/hicolor/scalable/apps/%{repo}.svg

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{repo}.desktop

%files
%doc README.md
%license LICENSE
%{_bindir}/%{repo}
%{_datadir}/%{repo}/
%{_datadir}/dbus-1/services/com.deepin.Calendar.service
%{_datadir}/applications/%{repo}.desktop
%{_datadir}/icons/hicolor/scalable/apps/%{repo}.svg

%changelog
