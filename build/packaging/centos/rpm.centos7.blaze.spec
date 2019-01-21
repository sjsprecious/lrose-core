%define build_timestamp %(date +"%Y%m%d")
%define _topdir     /root/rpmbuild
%define name        lrose 
%define release     %{build_timestamp}.centos7
%define version     blaze 
%define buildroot %{_topdir}/%{name}-%{version}-%{release}-root
 
BuildRoot:      %{_topdir}/installedhere
Summary:        LROSE
License:        BSD LICENSE
Name:           %{name}
Version:        %{version}
Release:        %{release}
Prefix:         /usr/local/lrose
Group:          Scientific Tools
AutoReqProv:    no
 
Requires: epel-release
Requires: libX11-devel, libXext-devel
Requires: libpng-devel, libtiff-devel, zlib-devel
Requires: expat-devel, libcurl-devel
Requires: flex-devel, fftw3-devel
Requires: bzip2-devel, qt5-qtbase-devel
Requires: hdf5-devel, netcdf-devel
Requires: xorg-x11-xauth, xorg-x11-apps

%description
LROSE - Lidar Radar Open Software Environment

# no build - it has been previously done and
# installed in /usr/local/lrose
 
%build

# We just need to install the files into place
# copy the install into buildroot/usr/local/lrose
# and make a listing of the files

%install
mkdir -p %{buildroot}%{prefix}
rsync -avL %{prefix}/* %{buildroot}%{prefix}
# find %{prefix} -type d > %{_topdir}/SPECS/lrose-pkg-files

# set the list of files
# they are just the files installed in buildroot

#%files -f %{_topdir}/SPECS/lrose-pkg-files
%files



