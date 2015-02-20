#pragma once

#include <map>

#include <QJsonObject>

#include <gfx/Camera.h>








class HouScene
{
public:
	typedef std::shared_ptr<HouScene> Ptr;
	HouScene();
	~HouScene();


	base::Camera::Ptr                         getCamera( const QString &name );



	void                                load( const QString &filename );
private:
	base::Camera::Ptr                         loadCamera( QJsonObject &obj );
	std::map<QString, base::Camera::Ptr>      m_cameras;
};
