#!/bin/bash

# Installs and configures HDFS.

die() {
  echo "$@" 1>&2 ; popd 2>/dev/null; exit 1
}


function update_apt_repo  {
  sudo apt-get install -y software-properties-common wget &&
    sudo apt-get purge -y openjdk* &&
    echo debconf shared/accepted-oracle-license-v1-1 select true | sudo debconf-set-selections &&
    echo debconf shared/accepted-oracle-license-v1-1 seen true | sudo debconf-set-selections &&
    sudo add-apt-repository -y ppa:webupd8team/java &&
    sudo apt-get update -y
    sudo apt-get install curl
} 

function install_java {
  sudo apt-get install -y oracle-java8-installer &&
    sudo apt-get install -y oracle-java8-set-default
}

function install_hadoop {
  sudo mkdir -p /usr/local/hadoop/ &&
    sudo chown -R $(whoami) /usr/local/hadoop || die "could not create local hadoop directory"
  pushd /usr/local/hadoop
  # download from closest mirror
  sudo curl -G -L -d "action=download" -d "filename=hadoop/common/hadoop-3.0.2/hadoop-3.0.2.tar.gz" \
	  https://www.apache.org/dyn/mirrors/mirrors.cgi -o hadoop-3.0.2.tar.gz
  if [ $? -ne 0 ]; then
    die "error downloading hadoop from apache mirror"
  fi;
  sudo tar xzf hadoop-3.0.2.tar.gz || die "error extracting hadoop download"
  if [ -d ./home/hadoop-3.0.2 ]; then
     sudo rm -rf ./home/hadoop-3.0.2
  fi
  sudo mv hadoop-3.0.2 home && sudo chown -R $(whoami) /usr/local/hadoop
  popd
}

function create_hadoop_user {
  sudo useradd -m hduser &&
    sudo adduser hduser sudo &&
    sudo chsh -s /bin/bash hduser
  echo -e "hduser123\nhduser123\n" | sudo passwd hduser

  sudo useradd -m hadoop &&
    sudo adduser hadoop sudo &&
    sudo chsh -s /bin/bash hadoop
  echo -e "hadoop123\nhadoop123\n" | sudo passwd hadoop
}

function setup_core_xml {
  export HADOOP_HOME=/usr/local/hadoop/home
  local tmpfile=/tmp/hadoop_fafsa.xml
  local file=$HADOOP_HOME/etc/hadoop/core-site.xml
  sudo rm -rf $file
  cat >> $tmpfile <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>
<configuration>
<property>
<name>hadoop.tmp.dir</name>
<value>/tmp/hadooop</value>
<description>Temporary directories.</description>
</property>
<property>
<name>fs.default.name</name>
<value>hdfs://localhost:9000</value>
</property>
</configuration>
EOF
  tmpfile=/tmp/hadoop_fafsa.xml
  mv $tmpfile $file
}

function setup_mapred_xml {
  export HADOOP_HOME=/usr/local/hadoop/home
  local tmpfile=/tmp/hadoop_mapred.xml
  local file=$HADOOP_HOME/etc/hadoop/mapred-site.xml
  sudo rm -rf $file
  cat >> $tmpfile <<EOT
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>
<configuration>
<property>
<name>mapred.job.tracker</name>
<value>localhost:9010</value>
<description>The tracker of MapReduce</description>
</property>
</configuration>
EOT
  tmpfile=/tmp/hadoop_mapred.xml
  mv $tmpfile $file
}

function setup_hdfs_xml {
  export HADOOP_HOME=/usr/local/hadoop/home
  local tmpfile=/tmp/hadoop_hdfs.xml
  local file=$HADOOP_HOME/etc/hadoop/hdfs-site.xml
  sudo rm -rf $file
  cat >> $tmpfile <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>
<configuration>

<property>
<name>dfs.replication</name>
<value>1</value>
</property>

<!-- libhdfs3 -->

<property>
<name>dfs.default.replica</name>
<value>1</value>
</property>

<property>
<name>output.replace-datanode-on-failure</name>
<value>false</value>
</property>

<property>
<name>dfs.client.read.shortcircuit</name>
<value>false</value>
</property>

<property>
<name>rpc.client.connect.retry</name>
<value>10</value>
</property>

<property>
<name>rpc.client.read.timeout</name>
<value>3600000</value>
</property>

<property>
<name>rpc.client.write.timeout</name>
<value>3600000</value>
</property>

</configuration>
EOF
  tmpfile=/tmp/hadoop_hdfs.xml
  mv $tmpfile $file
}


function setup_environment {
  export HADOOP_HOME=/usr/local/hadoop/home
  sudo sed -i -- 's/JAVA_HOME=\${JAVA_HOME}/JAVA_HOME=\$(readlink -f \/usr\/bin\/java | sed "s:bin\/java::")/' \
	  $HADOOP_HOME/etc/hadoop/hadoop-env.sh
  setup_core_xml &&
    setup_mapred_xml &&
    setup_hdfs_xml || die "error in generating xml configuration files"
}

function passwordless_ssh {
  if [ -d ~/.ssh ]; then
    rm -rf ~/.ssh
  fi
  sudo apt-get --reinstall install -y openssh-server openssh-client || die "error (re)installing openssh"
  mkdir ~/.ssh
  ssh-keygen -t rsa -P "" -f ~/.ssh/id_rsa
  cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
  chmod og-wx ~/.ssh/authorized_keys
  ssh-keyscan -H localhost >> ~/.ssh/known_hosts
  ssh-keyscan -H 0.0.0.0 >> ~/.ssh/known_hosts
  sudo service ssh restart || die "error restarting ssh service"
}

function run {
  update_apt_repo || die "error updating apt-repo"
  install_java || die "error installing java"
  create_hadoop_user || die "error creating hadoop user"
  install_hadoop || die "error installing hadoop"
  setup_environment || die "error setting up environment"
  passwordless_ssh || die "error setting up passwordless ssh"
}

run
