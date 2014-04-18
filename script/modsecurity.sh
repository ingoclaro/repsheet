install_mod_security () {
    if [ ! -d "vendor/modsecurity-apache_2.7.7" ]; then
        printf "$BLUE * $YELLOW Installing ModSecurity$RESET "
        pushd vendor > /dev/null 2>&1
        curl -s -O http://www.modsecurity.org/tarball/2.7.7/modsecurity-apache_2.7.7.tar.gz
        tar xzf modsecurity-apache_2.7.7.tar.gz
        printf "."
        pushd modsecurity-apache_2.7.7 > /dev/null 2>&1
        ./configure --prefix=$BUILDDIR/$APACHE_24_DIR \
            --exec-prefix=$BUILDDIR/$APACHE_24_DIR    \
            --libdir=$BUILDDIR/$APACHE_24_DIR/lib     \
            --with-apxs=$BUILDDIR/$APACHE_24_DIR/bin/apxs > install.log 2>&1
        printf "."
        make >> install.log 2>&1
        printf "."
        make install >> install.log 2>&1
        printf "."
        popd > /dev/null 2>&1
        popd > /dev/null 2>&1
        mkdir build/$APACHE_24_DIR/conf/modsecurity
        cp -r modsecurity/* build/$APACHE_24_DIR/conf/modsecurity/

        cat <<EOF >> build/$APACHE_24_DIR/conf/httpd.conf
LoadModule security2_module modules/mod_security2.so

<IfModule security2_module>
  Include conf/modsecurity/*.conf
</IfModule>

<IfModule repsheet_module>
  RepsheetEnabled On
  RepsheetRecorder On
  RepsheetFilter On
  RepsheetGeoIP On
  RepsheetProxyHeaders On
  RepsheetAction Notify
  RepsheetPrefix [repsheet]
  RepsheetAnomalyThreshold 20
  RepsheetRedisTimeout 5
  RepsheetRedisHost localhost
  RepsheetRedisPort 6379
  RepsheetRedisMaxLength 2
  RepsheetRedisExpiry 24
</IfModule>
EOF
        printf " $GREEN [Complete] $RESET\n"
    else
        printf "$BLUE * $GREEN ModSecurity already installed $RESET\n"
    fi
}
