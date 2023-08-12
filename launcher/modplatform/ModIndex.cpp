// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (c) 2022 flowln <flowlnlnln@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "modplatform/ModIndex.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QIODevice>

namespace ModPlatform {

static const QMap<QString, IndexedVersionType::Enum> s_indexed_version_type_names = { { "release", IndexedVersionType::Enum::Release },
                                                                                      { "beta", IndexedVersionType::Enum::Beta },
                                                                                      { "alpha", IndexedVersionType::Enum::Alpha } };

IndexedVersionType::IndexedVersionType(const QString& type) : IndexedVersionType(enumFromString(type)) {}

IndexedVersionType::IndexedVersionType(int type)
{
    switch (type) {
        case 1:
            m_type = IndexedVersionType::Enum::Release;
            break;
        case 2:
            m_type = IndexedVersionType::Enum::Beta;
            break;
        case 3:
            m_type = IndexedVersionType::Enum::Alpha;
            break;
        default:
            m_type = IndexedVersionType::Enum::UNKNOWN;
    }
}

IndexedVersionType::IndexedVersionType(const IndexedVersionType::Enum& type)
{
    m_type = type;
}

IndexedVersionType::IndexedVersionType(const IndexedVersionType& other)
{
    m_type = other.m_type;
}

IndexedVersionType& IndexedVersionType::operator=(const IndexedVersionType& other)
{
    m_type = other.m_type;
    return *this;
}

const QString IndexedVersionType::toString(const IndexedVersionType::Enum& type)
{
    switch (type) {
        case IndexedVersionType::Enum::Release:
            return "release";
        case IndexedVersionType::Enum::Beta:
            return "beta";
        case IndexedVersionType::Enum::Alpha:
            return "alpha";
        case IndexedVersionType::Enum::UNKNOWN:
        default:
            return "unknown";
    }
}

IndexedVersionType::Enum IndexedVersionType::enumFromString(const QString& type)
{
    return s_indexed_version_type_names.value(type, IndexedVersionType::Enum::UNKNOWN);
}

auto ProviderCapabilities::name(ResourceProvider p) -> const char*
{
    switch (p) {
        case ResourceProvider::MODRINTH:
            return "modrinth";
        case ResourceProvider::FLAME:
            return "curseforge";
    }
    return {};
}
auto ProviderCapabilities::readableName(ResourceProvider p) -> QString
{
    switch (p) {
        case ResourceProvider::MODRINTH:
            return "Modrinth";
        case ResourceProvider::FLAME:
            return "CurseForge";
    }
    return {};
}
auto ProviderCapabilities::hashType(ResourceProvider p) -> QStringList
{
    switch (p) {
        case ResourceProvider::MODRINTH:
            return { "sha512", "sha1" };
        case ResourceProvider::FLAME:
            // Try newer formats first, fall back to old format
            return { "sha1", "md5", "murmur2" };
    }
    return {};
}

auto ProviderCapabilities::hash(ResourceProvider p, QIODevice* device, QString type) -> QString
{
    QCryptographicHash::Algorithm algo = QCryptographicHash::Sha1;
    switch (p) {
        case ResourceProvider::MODRINTH: {
            algo = (type == "sha1") ? QCryptographicHash::Sha1 : QCryptographicHash::Sha512;
            break;
        }
        case ResourceProvider::FLAME:
            algo = (type == "sha1") ? QCryptographicHash::Sha1 : QCryptographicHash::Md5;
            break;
    }

    QCryptographicHash hash(algo);
    if (!hash.addData(device))
        qCritical() << "Failed to read JAR to create hash!";

    Q_ASSERT(hash.result().length() == hash.hashLength(algo));
    return { hash.result().toHex() };
}

QString getMetaURL(ResourceProvider provider, QVariant projectID)
{
    return ((provider == ModPlatform::ResourceProvider::FLAME) ? "https://www.curseforge.com/projects/" : "https://modrinth.com/mod/") +
           projectID.toString();
}

}  // namespace ModPlatform
