/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2015 ownCloud Inc
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsslcertificate.h"

#include <QtCore/qglobal.h>

#ifdef Q_OS_MACOS

#include "qtlsbackend_p.h"

#include <private/qcore_mac_p.h>

#include <QtCore/qdebug.h>

#include <CoreFoundation/CFArray.h>
#include <Security/Security.h>

#endif // Q_OS_MACOS

QT_BEGIN_NAMESPACE

#ifdef Q_OS_MACOS
namespace {

bool hasTrustedSslServerPolicy(SecPolicyRef policy, CFDictionaryRef props) {
    QCFType<CFDictionaryRef> policyProps = SecPolicyCopyProperties(policy);
    // only accept certificates with policies for SSL server validation for now
    if (CFEqual(CFDictionaryGetValue(policyProps, kSecPolicyOid), kSecPolicyAppleSSL)) {
        CFBooleanRef policyClient;
        if (CFDictionaryGetValueIfPresent(policyProps, kSecPolicyClient, reinterpret_cast<const void**>(&policyClient)) &&
            CFEqual(policyClient, kCFBooleanTrue)) {
            return false; // no client certs
        }
        if (!CFDictionaryContainsKey(props, kSecTrustSettingsResult)) {
            // as per the docs, no trust settings result implies full trust
            return true;
        }
        CFNumberRef number = static_cast<CFNumberRef>(CFDictionaryGetValue(props, kSecTrustSettingsResult));
        SecTrustSettingsResult settingsResult;
        CFNumberGetValue(number, kCFNumberSInt32Type, &settingsResult);
        switch (settingsResult) {
        case kSecTrustSettingsResultTrustRoot:
        case kSecTrustSettingsResultTrustAsRoot:
            return true;
        default:
            return false;
        }
    }
    return false;
}

bool isCaCertificateTrusted(SecCertificateRef cfCert, int domain)
{
    QCFType<CFArrayRef> cfTrustSettings;
    OSStatus status = SecTrustSettingsCopyTrustSettings(cfCert, SecTrustSettingsDomain(domain), &cfTrustSettings);
    if (status == noErr) {
        CFIndex size = CFArrayGetCount(cfTrustSettings);
        // if empty, trust for everything (as per the Security Framework documentation)
        if (size == 0) {
            return true;
        } else {
            for (CFIndex i = 0; i < size; ++i) {
                CFDictionaryRef props = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(cfTrustSettings, i));
                if (CFDictionaryContainsKey(props, kSecTrustSettingsPolicy)) {
                    if (hasTrustedSslServerPolicy((SecPolicyRef)CFDictionaryGetValue(props, kSecTrustSettingsPolicy), props))
                        return true;
                }
            }
        }
    } else {
        qCWarning(lcTlsBackend, "Error receiving trust for a CA certificate");
    }
    return false;
}

} // unnamed namespace
#endif // Q_OS_MACOS

namespace QTlsPrivate {
QList<QSslCertificate> systemCaCertificates()
{
    QList<QSslCertificate> systemCerts;
    // SecTrustSettingsCopyCertificates is not defined on iOS.
#ifdef Q_OS_MACOS
    // iterate through all enum members, order:
    // kSecTrustSettingsDomainUser, kSecTrustSettingsDomainAdmin, kSecTrustSettingsDomainSystem
    for (int dom = kSecTrustSettingsDomainUser; dom <= int(kSecTrustSettingsDomainSystem); dom++) {
        QCFType<CFArrayRef> cfCerts;
        OSStatus status = SecTrustSettingsCopyCertificates(SecTrustSettingsDomain(dom), &cfCerts);
        if (status == noErr) {
            const CFIndex size = CFArrayGetCount(cfCerts);
            for (CFIndex i = 0; i < size; ++i) {
                SecCertificateRef cfCert = (SecCertificateRef)CFArrayGetValueAtIndex(cfCerts, i);
                QCFType<CFDataRef> derData = SecCertificateCopyData(cfCert);
                if (isCaCertificateTrusted(cfCert, dom)) {
                    if (derData == nullptr) {
                        qCWarning(lcSsl, "Error retrieving a CA certificate from the system store");
                    } else {
                        systemCerts << QSslCertificate(QByteArray::fromCFData(derData), QSsl::Der);
                    }
                }
            }
        }
    }
#endif
    return systemCerts;
}
} // namespace QTlsPrivate

QT_END_NAMESPACE
