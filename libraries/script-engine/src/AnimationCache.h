//
//  AnimationCache.h
//  libraries/script-engine/src/
//
//  Created by Andrzej Kapolka on 4/14/14.
//  Copyright (c) 2014 High Fidelity, Inc. All rights reserved.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AnimationCache_h
#define hifi_AnimationCache_h

#include <ResourceCache.h>

#include <FBXReader.h>

class Animation;

typedef QSharedPointer<Animation> AnimationPointer;

/// Scriptable interface for FBX animation loading.
class AnimationCache : public ResourceCache {
    Q_OBJECT

public:

    AnimationCache(QObject* parent = NULL);

    Q_INVOKABLE AnimationPointer getAnimation(const QString& url) { return getAnimation(QUrl(url)); }
    
    Q_INVOKABLE AnimationPointer getAnimation(const QUrl& url);

protected:
    
    virtual QSharedPointer<Resource> createResource(const QUrl& url,
        const QSharedPointer<Resource>& fallback, bool delayLoad, const void* extra);
};

Q_DECLARE_METATYPE(AnimationPointer)

/// An animation loaded from the network.
class Animation : public Resource {
    Q_OBJECT

public:

    Animation(const QUrl& url);

    const FBXGeometry& getGeometry() const { return _geometry; }
    
    Q_INVOKABLE QStringList getJointNames() const;
    
    Q_INVOKABLE QVector<FBXAnimationFrame> getFrames() const;
    
protected:

    Q_INVOKABLE void setGeometry(const FBXGeometry& geometry);
    
    virtual void downloadFinished(QNetworkReply* reply);

private:
    
    FBXGeometry _geometry;
};

#endif // hifi_AnimationCache_h
