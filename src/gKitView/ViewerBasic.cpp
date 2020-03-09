#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>

#include <ViewerBasic.h>


using namespace std;

#define SCREEN_W 1024
#define SCREEN_H 768


/////////////////////////////
//! Gkit init functions 
/////////////////////////////  

ViewerBasic::ViewerBasic() : App(SCREEN_W, SCREEN_H), mb_cullface(true), mb_wireframe(false), b_draw_grid(true), b_draw_axe(true)
{
}


void ViewerBasic::help()
{
    printf("HELP:\n");
    printf("\th: help\n");
    printf("\tc: (des)active GL_CULL_FACE\n");
    printf("\tw: (des)active wireframe\n");
    printf("\ta: (des)active l'affichage de l'axe\n");
    printf("\tg: (des)active l'affichage de la grille\n");
    printf("\tz: (des)active l'affichage de la courbe d'animation\n");
    printf("\tfleches/pageUp/pageDown: bouge la camra\n");
    printf("\tCtrl+fleche/pageUp/pageDown: bouge la source de lumire\n");
    printf("\tSouris+bouton gauche: rotation\n");
    printf("\tSouris mouvement vertical+bouton droit: (de)zoom\n");
}



int ViewerBasic::init()
{
	SDL_SetWindowTitle(m_window, "gKit");
    cout<<"==>ViewerBasic"<<endl;

	int major = 0;
	int minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	cout << "OpenGL version " << major << " " << minor << endl;
	const GLubyte* txt;
	
	txt = glGetString(GL_VENDOR);
	if (txt) cout << "OpenGl Vendor "<< (const char*)txt<< endl;
	
	txt = glGetString(GL_RENDERER);
	if (txt) cout << "OpenGl Renderer " << (const char*)txt << endl;
	
	txt = glGetString(GL_VERSION);
	if (txt) cout << "OpenGl Version " << (const char*)txt << endl;

	txt = glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (txt) cout << "OpenGl Shading Language Version " << (const char*)txt << endl;

	// etat par defaut openGL
    glClearColor(0.13f, 0.13f, 0.13f, 1);
   
    glEnable(GL_DEPTH_TEST);

    m_camera.lookat( Point(0,0,0), 30 );
    m_camera.move(98);
    
    gl.light( Point(0, 20, 20), White() );

    init_axe();
    init_grid();
    init_cube();
    init_quad();

    m_tex_debug = read_texture(3, "../data/debug2x2red.png");
    // OpenCV & dLib
    
    loadFaceDetectionModels();
    initFBO(texID);
    initCvCapture();
    
    val=0.0;
    offsetNum = 0.0;
    //! Blendshapes
    init_BSShader();
    
    //                 [0]         [1]
    //               w0   w1     w0   w1
    tab_weights = {{0.0, 0.0}, {0.0, 0.0}};


    // 3D model points.
    model_points.push_back(cv::Point3d(0.0f, 0.0f, 0.0f));            // Nose tip
    model_points.push_back(cv::Point3d(0.0f, -33.0f, -6.5f));         // Chin
    model_points.push_back(cv::Point3d(-22.5f, 17.0f, -13.5f));       // Left eye left corner
    model_points.push_back(cv::Point3d(22.5f, 17.0f, -13.5f));        // Right eye right corner
    model_points.push_back(cv::Point3d(-15.0f, -15.0f, -12.5f));      // Left Mouth corner
    model_points.push_back(cv::Point3d(15.0f, -15.0f, -12.5f));       // Right mouth corner
    
    return 1;
}

int ViewerBasic::initFBO(GLuint &id){
    // setup FBO
    glGenFramebuffers(1, &fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
}

void ViewerBasic::init_axe()
{
    m_axe = Mesh(GL_LINES);
    m_axe.color( Color(1, 0, 0));
    m_axe.vertex( 0,  0, 0);
    m_axe.vertex( 1,  0, 0);

    m_axe.color( Color(0, 1, 0));
    m_axe.vertex( 0,  0, 0);
    m_axe.vertex( 0,  1, 0);

    m_axe.color( Color( 0, 0, 1));
    m_axe.vertex( 0,  0, 0);
    m_axe.vertex( 0,  0, 1);
}

void ViewerBasic::init_grid()
{
    m_grid = Mesh(GL_LINES);

    m_grid.color( Color(1, 1, 1));
    int i,j;
    for(i=-5;i<=5;++i)
        for(j=-5;j<=5;++j)
        {
            m_grid.vertex( -5, 0, j);
            m_grid.vertex( 5, 0,  j);

            m_grid.vertex( i, 0, -5);
            m_grid.vertex( i, 0, 5);

        }
}


void ViewerBasic::init_cube()
{
    //                          0           1           2       3           4           5       6           7
    static float pt[8][3] = { {-1,-1,-1}, {1,-1,-1}, {1,-1,1}, {-1,-1,1}, {-1,1,-1}, {1,1,-1}, {1,1,1}, {-1,1,1} };
    static int f[6][4] = {    {0,1,2,3}, {5,4,7,6}, {2,1,5,6}, {0,3,7,4}, {3,2,6,7}, {1,0,4,5} };
    static float n[6][3] = { {0,-1,0}, {0,1,0}, {1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1} };
    int i;

    m_cube = Mesh(GL_TRIANGLE_STRIP);
    m_cube.color( Color(1, 1, 1) );

    for (i=0;i<6;i++)
    {
        m_cube.normal(  n[i][0], n[i][1], n[i][2] );

        m_cube.texcoord( 0,0 );
        m_cube.vertex( pt[ f[i][0] ][0], pt[ f[i][0] ][1], pt[ f[i][0] ][2] );

        m_cube.texcoord( 1,0);
        m_cube.vertex( pt[ f[i][1] ][0], pt[ f[i][1] ][1], pt[ f[i][1] ][2] );

        m_cube.texcoord(0,1);
        m_cube.vertex(pt[ f[i][3] ][0], pt[ f[i][3] ][1], pt[ f[i][3] ][2] );

        m_cube.texcoord(1,1);
        m_cube.vertex( pt[ f[i][2] ][0], pt[ f[i][2] ][1], pt[ f[i][2] ][2] );

        m_cube.restart_strip();
    }
}

void ViewerBasic::init_quad()
{
    m_quad = Mesh(GL_TRIANGLE_STRIP);
    m_quad.color( Color(1, 1, 1));

    m_quad.normal(  0, 0, 1 );

    m_quad.texcoord(0,0 );
    m_quad.vertex(-1, -1, 0 );

    m_quad.texcoord(1,0);
    m_quad.vertex(  1, -1, 0 );

    m_quad.texcoord(0,1);
    m_quad.vertex( -1, 1, 0 );

    m_quad.texcoord( 1,1);
    m_quad.vertex(  1,  1, 0 );
}

Mesh ViewerBasic::init_OBJ(const char *filename){
    return read_mesh(filename);
}


/////////////////////////////
//! Gkit Render functions 
/////////////////////////////  
int ViewerBasic::render()
{
    // thread t1(&ViewerBasic::doCvCapture, this, std::ref(this->cvMatCam));
    
    // Efface l'ecran
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Deplace la camera, lumiere, etc.
    manageCameraLight();

    // Donne notre camera au shader
    gl.camera(m_camera);

    // Lance la capture webcam avec openCV
    doCvCapture(cvMatCam);
    
    draw_quad(Scale(5,5,5)*Translation(10,0,0), m_tex_debug);

    draw_blendshapes();

    
    //renderToFBO(cvMatCam);

    // t1.join();
    


    return 1;
}

int ViewerBasic::renderToFBO(cv::Mat& cvImage){

    glBindFramebuffer(GL_FRAMEBUFFER, fboID);
    //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    
    // glBindTexture(GL_TEXTURE_2D, texID);

    // fixe les parametres de filtrage par defaut
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // transfere les donnees dans la texture
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 cvImage.cols,
                 cvImage.rows,
                 0,
                 GL_BGR,
                 GL_UNSIGNED_BYTE,
                 cvImage.ptr());

    // prefiltre la texture
    glGenerateMipmap(GL_TEXTURE_2D);

    
   // glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 0;
    
}

/////////////////////////////
//! Gkit draw functions
/////////////////////////////  

void ViewerBasic::draw(const Transform& T,  Mesh &mesh ){
    gl.model(T);
    gl.texture(0);
    gl.lighting(false);
    gl.draw(mesh);
    gl.lighting(true);

}

void ViewerBasic::draw_axe(const Transform& T)
{
    gl.model(T);
    gl.texture(0);
    gl.lighting(false);
    gl.draw(m_axe);
    gl.lighting(true);
}


void ViewerBasic::draw_grid(const Transform& T)
{
	gl.lighting(false);
	gl.texture(0);
	gl.model(T);
	gl.draw(m_grid);
}

void ViewerBasic::draw_cube(const Transform& T)
{
	gl.lighting(true);
	gl.texture(0);
	gl.model(T);
	gl.draw(m_cube);
}

void ViewerBasic::draw_quad(const Transform& T, const GLuint &Tex)
{
	gl.lighting(true);
	gl.texture(Tex);
	gl.model(T);
	gl.draw(m_quad);
}

/////////////////////////////
//! Gkit camera functions
///////////////////////////// 

void ViewerBasic::manageCameraLight()
{
    // recupere les mouvements de la souris pour deplacer la camera, cf tutos/tuto6.cpp
    int mx, my;
    unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
    // deplace la camera
    if((mb & SDL_BUTTON(1)) &&  (mb& SDL_BUTTON(3)))                 // le bouton du milieu est enfonce
        m_camera.translation( (float) mx / (float) window_width(), 
                              (float) my / (float) window_height());         // deplace le point de rotation
    else if(mb & SDL_BUTTON(1))                      // le bouton gauche est enfonce
        m_camera.rotation( mx, my);       // tourne autour de l'objet
    else if(mb & SDL_BUTTON(3))                 // le bouton droit est enfonce
        m_camera.move( my);               // approche / eloigne l'objet
	if (key_state(SDLK_PAGEUP) && (!key_state(SDLK_LCTRL)) && (!key_state(SDLK_LALT))) { m_camera.translation(0, 0.01); }
	if (key_state(SDLK_PAGEDOWN) && (!key_state(SDLK_LCTRL)) && (!key_state(SDLK_LALT))) { m_camera.translation(0, -0.01); }
	if (key_state(SDLK_LEFT) && (!key_state(SDLK_LCTRL)) && (!key_state(SDLK_LALT))) { m_camera.translation(0.01, 0); }
	if (key_state(SDLK_RIGHT) && (!key_state(SDLK_LCTRL)) && (!key_state(SDLK_LALT))) { m_camera.translation(-0.01, 0); }
	if (key_state(SDLK_UP) && (!key_state(SDLK_LCTRL)) && (!key_state(SDLK_LALT))) { m_camera.move(1); }
	if (key_state(SDLK_DOWN) && (!key_state(SDLK_LCTRL)) && (!key_state(SDLK_LALT))) { m_camera.move(-1); }


    // Deplace la lumiere
    const float step = m_camera.radius()*0.005f;
    if (key_state(SDLK_RIGHT) && key_state(SDLK_LCTRL)) { gl.light( gl.light()+Vector(step,0,0)); }
    if (key_state(SDLK_LEFT) && key_state(SDLK_LCTRL)) { gl.light( gl.light()+Vector(-step,0,0)); }
    if (key_state(SDLK_UP) && key_state(SDLK_LCTRL)) { gl.light( gl.light()+Vector(0,0,-step)); }
    if (key_state(SDLK_DOWN) && key_state(SDLK_LCTRL)) { gl.light( gl.light()+Vector(0,0,step)); }
    if (key_state(SDLK_PAGEUP) && key_state(SDLK_LCTRL)) { gl.light( gl.light()+Vector(0,step,0)); }
    if (key_state(SDLK_PAGEDOWN) && key_state(SDLK_LCTRL)) { gl.light( gl.light()+Vector(0,-step,0)); }



    // (De)Active la grille / les axes
    if (key_state('h')) help();
    if (key_state('c')) { clear_key_state('c'); mb_cullface=!mb_cullface; if (mb_cullface) glEnable(GL_CULL_FACE);else glDisable(GL_CULL_FACE); }
    if (key_state('w')) { clear_key_state('w'); mb_wireframe=!mb_wireframe; if (mb_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
    if (key_state('g')) { b_draw_grid = !b_draw_grid; clear_key_state('g'); }
    if (key_state('a')) { b_draw_axe = !b_draw_axe; clear_key_state('a'); }

    gl.camera(m_camera);
    //draw(cube, Translation( Vector( gl.light()))*Scale(0.3, 0.3, 0.3), camera);
    //draw_param.texture(quad_texture).camera(camera).model(Translation( 3, 5, 0 )).draw(quad);

    // AXE et GRILLE
    gl.model( Scale(10.*step,10.0*step,10.0*step) );
    if (b_draw_grid) gl.draw(m_grid);
    if (b_draw_axe) gl.draw(m_axe);

    //  LIGHT
    gl.texture( 0 );
    gl.lighting(false);
	gl.model(Translation(Vector(gl.light()))*Scale(step, step, step));
    gl.draw(m_cube);
    gl.lighting(true);
}

/////////////////////////////
//! openCV & dlib
///////////////////////////// 


int ViewerBasic::initCvCapture(){
    cap = cv::VideoCapture(0); 
    
    faceDetected = false;
    cap.set(CV_CAP_PROP_BRIGHTNESS, .5); 
    
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 512);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 512);
    if(!cap.isOpened()){
        cerr << "Unable to connect to camera" << endl;
        return 1;
    }
    
    return 0;
    
}

void ViewerBasic::loadFaceDetectionModels(){
    // Load face detection and pose estimation models.
    std::cout << "dlib detector load\n";

    try
    {
        detector = dlib::get_frontal_face_detector();
        dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;
    }
    catch (dlib::serialization_error &e)
    {
        cout << "You need dlib's default face landmarking model file to run this example." << endl;
        cout << "You can get it from the following URL: " << endl;
        cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
        cout << endl
             << e.what() << endl;
    }
    catch (exception &e)
    {
        cout << e.what() << endl;
    }

    
    
}

void ViewerBasic::computePnP(){

    using namespace dlib;
    using namespace cv;
    
    //need to make sure that there is at least 1 face present to use the PnP function
    if(faceDetected){

        image_points.push_back( currentPose[33] );    // Nose tip
        image_points.push_back( currentPose[8]  );    // Chin
        image_points.push_back( currentPose[45] );    // Left eye left corner
        image_points.push_back( currentPose[36] );    // Right eye right corner
        image_points.push_back( currentPose[54] );    // Left Mouth corner
        image_points.push_back( currentPose[48] );    // Right mouth corner
        
        //PnP functions
        // Camera internals
        double focal_length = cvMatCam.cols; // Approximate focal length.
        Point2d center = cv::Point2d(cvMatCam.cols/2,cvMatCam.rows/2);
        cv::Mat camera_matrix = (cv::Mat_<double>(3,3) << focal_length, 0, center.x, 0 , focal_length, center.y, 0, 0, 1);
        cv::Mat dist_coeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type); // Assuming no lens distortion
        
        //cout << "Camera Matrix " << endl << camera_matrix << endl ;
         rotation_vector; // Rotation in axis-angle form
         translation_vector;

         std::vector<cv::Mat> rotation_mean;
        
        // Solve for pose
        solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);

        
        
        // Project a 3D point (0, 0, 1000.0) onto the image plane.
        // We use this to draw a line sticking out of the nose
        
        std::vector<Point3d> nose_end_point3D;
        std::vector<Point2d> nose_end_point2D;
        nose_end_point3D.push_back(Point3d(0,0,1000));
        
        projectPoints(nose_end_point3D, rotation_vector, translation_vector, camera_matrix, dist_coeffs, nose_end_point2D);
        
        
        for(int i=0; i < image_points.size(); i++)
        {
            circle(cvMatCam, image_points[i], 3, Scalar(0,0,255), -1);
        }
        
        cv::line(cvMatCam,image_points[0], nose_end_point2D[0], cv::Scalar(255,0,0), 2);
        
        transformModel = Translation(translation_vector.at<double>(0)/1000 , -translation_vector.at<double>(1)/1000 , translation_vector.at<double>(2)/1000); 
        // rotationModel = RotationX(rotation_vector.at<double>(0)*180/M_PI) * RotationY(rotation_vector.at<double>(1)*180/M_PI) * RotationZ(rotation_vector.at<double>(2)*180/M_PI);

        cout << "Rotation Vector " << rotation_vector << endl;

        // cout << "Rotation Vector " << rotation_vector.size() << endl;

        cout << "Rotation Vector 0 => " << rotation_vector.at<double>(0)*180/M_PI << endl;
        cout << "Rotation Vector 1 => " << rotation_vector.at<double>(1)*180/M_PI << endl;
        cout << "Rotation Vector 2 => " << rotation_vector.at<double>(2)*180/M_PI << endl;
        // //cout << "Translation Vector" << endl << translation_vector << endl;

        // cout << "translation_vector axe z => " << 2-(translation_vector.at<double>(2)/1000) << endl;
        
        //cout <<  nose_end_point2D << endl;

    }else{
        // if there is no detected face, then reset the models' position
        transformModel = Translation(0,0,0) * Scale(0.3, 0.3, 0.3);
        rotationModel = RotationX(0) * RotationY(0) * RotationZ(0);
    }
    image_points.clear();
}

int ViewerBasic::doCvCapture(cv::Mat &out)
{
    using namespace dlib;
    using namespace cv;

    // Lance la capture webcam et stocke le résultat dans une matrice openCV (cv::Mat)
    try
    {
          
        // Grab a frame
        if (!cap.read(cvMatCam))
        {
            return -1;
        }
        

        cv_image<bgr_pixel> cimg(cvMatCam);
        
        // Detect faces
        std::vector<dlib::rectangle> faces = detector(cimg);;
        std::vector<dlib::full_object_detection> shapes;
        
        // Find the pose of each face.
        faceDetected = faces.size() > 0;

        for (unsigned long i = 0; i < faces.size(); ++i){
            shapes.push_back(pose_model(cimg, faces[i]));
            
            cv::Rect boundingBox = cv::Rect2f(faces.at(i).left(), faces.at(i).top(), faces.at(i).width(),  faces.at(i).height());
            cv::rectangle(cvMatCam, boundingBox, cv::Scalar(255,0,0), 2);
            

        }

        getPose(shapes, currentPose);
        for(unsigned int i=0 ; i < currentPose.size() ; i++){
            cv::Point2i keyPoint = cv::Point2i(currentPose.at(i).x,currentPose.at(i).y );
            drawMarker(cvMatCam, keyPoint, cv::Scalar(0,0,255), 0, 11, 1);  
        }

        // computePnP();
        
        

    if(faceDetected){
            // Capture des expression
        if(key_state(SDLK_1)){

                std::cout << "[saving neutral pose...]\n";

                //stockage des poids
                getPose(shapes, p_neutral);
                tab_weights[0][0] = 100.0;
                tab_weights[0][1] = 0.0;
                std::cout << "[weights for neutral pose saved !]\n";
                
                displayTab2D(tab_weights);

        }

        if (key_state(SDLK_2))
        {
            std::cout << "[saving jaw open pose]\n";
            getPose(shapes, p_jawOpen);
            tab_weights[1][0] = 0.0;
            tab_weights[1][1] = 100.0;
            std::cout << "[weights for mouth open pose saved !]\n";

            displayTab2D(tab_weights);
        }

        if (key_state(SDLK_3))
        {
            std::cout << "[saving jaw left pose]\n";
            getPose(shapes, p_jawLeft);
            tab_weights[1][0] = 0.0;
            tab_weights[1][1] = 100.0;
            std::cout << "[weights for mouth open pose saved !]\n";

            displayTab2D(tab_weights);
        }

        if (key_state(SDLK_4))
        {
            std::cout << "[saving  jaw right pose]\n";
            getPose(shapes, p_jawRight);
            tab_weights[1][0] = 0.0;
            tab_weights[1][1] = 100.0;
            std::cout << "[weights for mouth open pose saved !]\n";

            displayTab2D(tab_weights);
        }

        if (key_state(SDLK_5))
        {
            std::cout << "[saving eyebrows up]\n";
            getPose(shapes, p_eyeBrowsRaised);
            tab_weights[1][0] = 0.0;
            tab_weights[1][1] = 100.0;
            std::cout << "[weights for mouth open pose saved !]\n";

            displayTab2D(tab_weights);
        }


        

        //TODO aligner les point capturés d'une pose avec la pose actuellement capturée
        if (!p_jawOpen.empty())//&& !p_jawLeft.empty() )
        {
            
            w_neutral = computeWeight(currentPose, p_neutral);
            w_jawOpen = computeWeight(currentPose, p_jawOpen);
            w_jawLeft = computeWeight(currentPose, p_jawLeft);
            w_jawRight = computeWeight(currentPose, p_jawRight);
            w_eyeBrowsRaised = computeWeight(currentPose, p_eyeBrowsRaised); 
            
        }

        faceKeyPoints.clear();
        currentPose.clear();

        // Display it all on the screen
        win.clear_overlay();
        win.set_image(cimg);
        win.add_overlay(render_face_detections(shapes));
    }




    }
    catch (exception &e) 
    {
        cout << e.what() << endl;
    }

    
    return 0;
}

 double ViewerBasic::computeWeight(std::vector<cv::Point2f> currentPose, std::vector<cv::Point2f> expression ){
            double sum_w, sum_dist = 0;
            double weight = 0;
            for (unsigned int i = 0; i < 68; i++)
            {
                double x_offset = currentPose.at(27).x - expression.at(27).x;
                double y_offset = currentPose.at(27).y - expression.at(27).y;

                double dist = distance(cv::Point2d(currentPose.at(i).x, currentPose.at(i).y),
                                       cv::Point2d(expression.at(i).x + x_offset, expression.at(i).y + y_offset));

                // smooth
                dist = pow(dist, 0.19);

                sum_dist += dist;
                if (dist == 0.0) dist = 0.01;
                    
                sum_w += 1 / (dist);

                weight = (sum_w / (sum_dist)) - 0.2;
                if (weight > 1.0) weight = 1.0;
                if (weight < 0.1) weight = 0.01;

                // std::cout << "Sum of distances | Sum of Weights | Weight value : " << sum_dist << " | " << sum_w << " | " << val << "\n";
                drawMarker(cvMatCam, cv::Point2d(expression.at(i).x + x_offset, expression.at(i).y + y_offset) , cv::Scalar(255, 255, 0), 0, 10);
                sum_w = 0;
                sum_dist = 0;
            }

            return weight;

}



double ViewerBasic::distance(cv::Point2f a,cv::Point2f b ){
    return sqrtf( std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2) );
}


void ViewerBasic::dlibDrawText(const dlib::point &p, const string &s){
    win.add_overlay(dlib::image_window::overlay_rect(dlib::rectangle(p), dlib::rgb_pixel(255,255,0), s));
}

void ViewerBasic::displayTab2D(std::vector<std::vector<double>> tab_weights){
            for(int i=0; i < tab_weights.size() ; i++){
                for(int j=0; j < tab_weights[i].size(); j++){
                    std::cout << "[" << i << "," << j << "] : " << tab_weights.at(i).at(j) << std::endl;
                }
            }
}

void ViewerBasic::getPose(std::vector<dlib::full_object_detection> shapes, std::vector<cv::Point2f> &out)
{
    
    // Récupère les coordonnées des point caractéristique du visage (2D)
    if(faceDetected){
        
        for (unsigned long j = 0; j < 68; j++)
        {
            cv::Point2f point(shapes[0].part(j).x(), shapes[0].part(j).y());
            out.push_back(point);            
        }
        // std::cout <<"[points saved]\n";
    }
         
}


////////////////////////////
//! Blendshapes functions
////////////////////////////
void ViewerBasic::init_BSShader(){
    // //! https://perso.univ-lyon1.fr/jean-claude.iehl/Public/educ/M1IMAGE/html/group__tuto__mesh__buffer.html
    program = 0;
    program = read_program("../data/shaders/blendshape.glsl");
    program_print_errors(program);

    //! chargement des differentes poses 
    m_neutral = read_mesh("../data/blendshapes/Neutral.obj");
    m_jawOpen = read_mesh("../data/blendshapes/jawOpen.obj");
    m_jawLeft = read_mesh("../data/blendshapes/mouthSmileLeft.obj");
    m_jawRight = read_mesh("../data/blendshapes/mouthSmileRight.obj");
    m_eyeBrowsRaised = read_mesh("../data/blendshapes/browInnerUp.obj");

    //! Initialisation du tableau de position clés 
    getModelKeyPoints();

    if(m_neutral.normal_buffer_size() == 0)
        std::cout << "ERREUR, pas de vertex normals...";

    std::vector<Mesh> tabMesh;
    tabMesh.push_back(m_neutral);
    tabMesh.push_back(m_jawOpen);
    tabMesh.push_back(m_jawLeft);
    tabMesh.push_back(m_jawRight);
    tabMesh.push_back(m_eyeBrowsRaised);

    // cree un VAO qui va contenir la position des sommet de nos mesh 
    mVA1.create(tabMesh);
    m_neutral.release(); 
    m_jawOpen.release();
    m_jawLeft.release();
    m_jawRight.release();
    m_eyeBrowsRaised.release();

    
}

void ViewerBasic::draw_blendshapes(){
    //pour l'instant, les obj n'ont pas de vertex normal/color/texcoord 

    glUseProgram(program);

    Transform model = Identity() * Scale(1,1,1);
    
    Transform view = m_camera.view();
    Transform projection = m_camera.projection(window_width(), window_height(), 45);

    Transform mv = m_camera.view() * model * transformModel * rotationModel;
    mvp = projection * mv;

    program_uniform(program, "normalMatrix", mv.normal()); // transforme les normales dans le repere camera.
    program_uniform(program, "mvpMatrix", mvp);
    program_uniform(program, "mvMatrix", mv);
    
    program_uniform(program, "mesh_color", m_neutral.default_color());
    program_uniform(program, "color", Red());

    program_uniform(program, "light", view(gl.light()));
    

    //weights

    

    // if (key_state(SDLK_UP)) val += 1.0;
        program_uniform(program, "w_jawOpen", w_jawOpen);
        program_uniform(program, "w_jawLeft", w_jawLeft);
        program_uniform(program, "w_jawRight", w_jawRight);
        program_uniform(program, "w_eyeBrowsRaised", w_eyeBrowsRaised);
        
    

    // On selection notre VAO pour le vertex shader
    glBindVertexArray(mVA1.vao);
    
    
    glDrawArrays(GL_TRIANGLES, 0, mVA1.vertex_count);
  
}


void ViewerBasic::getModelKeyPoints(){
    // Initialise le tableau modelKeyPoints avec les position des sommets clés

    // Positions pour la pose Neutre

    //-----  Left Eye v1077   -----//
    modelKeyPoints.push_back(cv::Point3f(0.0312433, 0.0286743, 0.0420298));

    //-----  Left Right Eye v1095   -----//
    modelKeyPoints.push_back(cv::Point3f(-0.0312433, 0.0286743, 0.0420298));

    //-----  Left Ear v1095  -----//
    modelKeyPoints.push_back(cv::Point3f(-0.0312433, 0.0286743, 0.0420298));

    //-----    Nose   v8     -----//
    modelKeyPoints.push_back(cv::Point3f(0.0, -0.00980007, 0.0778864));

    //-----  Right Ear v58   -----//
    modelKeyPoints.push_back(cv::Point3f(-0.0716188, 0.0130347, -0.0271589));

    //-----  Left Mouth v640 -----//
    modelKeyPoints.push_back(cv::Point3f(0.0246458, -0.0419488, 0.0466527));

    //-----  Right Mouth v189 -----//
    modelKeyPoints.push_back(cv::Point3f(-0.0262095, -0.0423298, 0.0460978));

}
