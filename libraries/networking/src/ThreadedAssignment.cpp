//
//  ThreadedAssignment.cpp
//  libraries/shared/src
//
//  Created by Stephen Birarda on 12/3/2013.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QtCore/QCoreApplication>
#include <QtCore/QJsonObject>
#include <QtCore/QThread>
#include <QtCore/QTimer>

#include <LogHandler.h>

#include "ThreadedAssignment.h"

ThreadedAssignment::ThreadedAssignment(NLPacket& packet) :
    Assignment(packet),
    _isFinished(false)

{

}

void ThreadedAssignment::setFinished(bool isFinished) {
    if (_isFinished != isFinished) {
         _isFinished = isFinished;

        if (_isFinished) {

            qDebug() << "ThreadedAssignment::setFinished(true) called - finishing up.";

            if (_domainServerTimer) {
                _domainServerTimer->stop();
            }

            if (_statsTimer) {
                _statsTimer->stop();
            }

            // call our virtual aboutToFinish method - this gives the ThreadedAssignment subclass a chance to cleanup
            aboutToFinish();

            emit finished();
        }
    }
}

void ThreadedAssignment::commonInit(const QString& targetName, NodeType_t nodeType, bool shouldSendStats) {
    // change the logging target name while the assignment is running
    LogHandler::getInstance().setTargetName(targetName);

    auto nodeList = DependencyManager::get<NodeList>();
    nodeList->setOwnerType(nodeType);

    _domainServerTimer = new QTimer();
    connect(_domainServerTimer, SIGNAL(timeout()), this, SLOT(checkInWithDomainServerOrExit()));
    _domainServerTimer->start(DOMAIN_SERVER_CHECK_IN_MSECS);

    if (shouldSendStats) {
        // send a stats packet every 1 second
        _statsTimer = new QTimer();
        connect(_statsTimer, &QTimer::timeout, this, &ThreadedAssignment::sendStatsPacket);
        _statsTimer->start(1000);
    }
}

void ThreadedAssignment::addPacketStatsAndSendStatsPacket(QJsonObject &statsObject) {
    auto nodeList = DependencyManager::get<NodeList>();

    float packetsPerSecond, bytesPerSecond;
    // XXX can BandwidthRecorder be used for this?
    nodeList->getPacketStats(packetsPerSecond, bytesPerSecond);
    nodeList->resetPacketStats();

    statsObject["packets_per_second"] = packetsPerSecond;
    statsObject["bytes_per_second"] = bytesPerSecond;

    nodeList->sendStatsToDomainServer(statsObject);
}

void ThreadedAssignment::sendStatsPacket() {
    QJsonObject statsObject;
    addPacketStatsAndSendStatsPacket(statsObject);
}

void ThreadedAssignment::checkInWithDomainServerOrExit() {
    if (DependencyManager::get<NodeList>()->getNumNoReplyDomainCheckIns() == MAX_SILENT_DOMAIN_SERVER_CHECK_INS) {
        setFinished(true);
    } else {
        DependencyManager::get<NodeList>()->sendDomainServerCheckIn();
    }
}
