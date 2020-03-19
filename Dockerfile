FROM centos:latest
COPY bin/i386/ /opt/eagleowl
COPY etc/eagleowl.conf /etc
RUN yum install -y httpd libstdc++.i686 libusb.i686
COPY www /var/www
RUN sed -i 's|main_db_name.*|main_db_name=/opt/eagleowl/data/eagleowl.db|' /etc/eagleowl.conf
RUN sed -i 's|stat_db_name.*|stat_db_name=/opt/eagleowl/data/eagleowl_stat.db|' /etc/eagleowl.conf
VOLUME /opt/eagleowl/data
WORKDIR /opt/eagleowl/data
ENTRYPOINT ["/opt/eagleowl/cm160"]
