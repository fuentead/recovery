#######################################################################
# @@@ START COPYRIGHT @@@
#
# (C) Copyright 2007-2014 Hewlett-Packard Development Company, L.P.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
# @@@ END COPYRIGHT @@@
#######################################################################

CPPSRC := \
	dummy.cpp

SRCPATH := security/spj

INCLUDE_DIRS := security/spj

JSRC := SecSpjUtil.java usermgtspj.java ldapconfigspj.java SelfSignedCertificate.java SecurityPolicy.java CustomCertificate.java X509CertificateGenerator.java SecConfig.java

#-- This assumes no PACKAGE structure
#-- All JSRC files are compiled to $(TARGOBJDIR)/java
PACKAGE := .
JARPREFIX := securityspj
CLASSPATH :=../security/spj/;$(TARGOBJDIR)/java/;../security/spj/securityspj.jar
JAR_APPEND := ../security/spj/securityspj.jar

DEFS := -Dexport_SecurityException -DNA_NSK
