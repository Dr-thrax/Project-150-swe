#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <string> 
#include <cstdlib>



using namespace std;
using namespace sf;

 constexpr float ballRadius{ 10.0f }, ballVelocity{ 8.f };
constexpr int WindowHeight{ 720 }, WindowWidth{ 950 };
constexpr  float paddleWidth{ 120.0f }, paddleHeight{ 10.0f }, paddleVelocity{16.f};
constexpr float blockWidth{ 60.f }, blockHeight{ 20.f };
constexpr int countBlocksx{ 14 }, countBlocksy{ 4 };

bool isGameOver = false;



int score{ 0 };
int level{ 1 };
int button{ 0 };

Music music;
Music music2;

Texture paddleTexture;
Texture BallTexture;
Texture brickTexture;
Texture loading;
Texture texture;

Sprite sprite;


Text points;
Text lvltxt;
Font font;
bool stopmusic = false;


struct Ball
{
	CircleShape shape;
	
	Vector2f velocity{ -ballVelocity,-ballVelocity };

	Ball(float mx, float my)
	{
		shape.setPosition(mx, my);
		shape.setRadius(ballRadius);
		shape.setFillColor(Color::White);
		shape.setOrigin(ballRadius, ballRadius);
		shape.setFillColor(Color::Yellow);

	}

	void update() 
	{
		if(top() < WindowHeight )shape.move(velocity);
		else {
			shape.move(0.f, 0.f);
			isGameOver = true;
		}

		if (left() < 0) velocity.x = ballVelocity;
		else if (right() > WindowWidth) velocity.x = -ballVelocity;
		if (top() < 0) velocity.y = ballVelocity;
		
		// else if (bottom() > WindowHeight) velocity.y = -ballVelocity;

		
	}

	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }
	float left() { return x() - shape.getRadius(); }
	float right() { return x() + shape.getRadius(); }
	float top() { return y() - shape.getRadius(); }
	float bottom() { return y() + shape.getRadius(); }
};

struct paddle
{
	RectangleShape shape;
	Vector2f velocity;

	paddle(float mx, float my)
	{
		shape.setPosition(mx, my);
		shape.setSize({ paddleWidth,paddleHeight });
		shape.setFillColor(Color::White);
		shape.setOrigin(paddleWidth / 2.f, paddleHeight / 2.f);
	}

	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }
	float left() { return x()   -  shape.getSize().x / 2.0f; }
	float right() { return x()  +  shape.getSize().x / 2.0f; }
	float top() { return y()    -  shape.getSize().y / 2.0f; }
	float bottom() { return y() +  shape.getSize().y / 2.0f; }

	void update()
	{
		shape.move(velocity);


		if (Keyboard::isKeyPressed(Keyboard::Key::Left) && left() > 0) velocity.x = -paddleVelocity;
		else if (Keyboard::isKeyPressed(Keyboard::Key::Right) && right() < WindowWidth) velocity.x = paddleVelocity;
		else velocity.x = 0;
 	}
};


struct Brick 
{
	RectangleShape shape;

	bool destroyed{ false };

	Brick(float mX, float mY)
	{
		shape.setPosition(mX, mY);
		shape.setSize({ blockWidth, blockHeight });
		
		shape.setOrigin(blockWidth / 2.f, blockHeight / 2.f);

	}

	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }
	float left() { return x() - shape.getSize().x / 2.0f; }
	float right() { return x() + shape.getSize().x / 2.0f; }
	float top() { return y() - shape.getSize().y / 2.0f; }
	float bottom() { return y() + shape.getSize().y / 2.0f; }



};

template<class T1, class T2> bool isIntersecting(T1& mA, T2& mB)
{
	return mA.right() >= mB.left() && mA.left() <= mB.right() 
		&& mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}


void testCollision(paddle& mpaddle, Ball& mBall)
{
	if (!isIntersecting(mpaddle, mBall)) return;

	mBall.velocity.y = -ballVelocity;
	music2.openFromFile("boink3.wav");
	music2.setVolume(100);
	if(!stopmusic) music2.play();

	if (mBall.x() < mpaddle.x()) mBall.velocity.x = -ballVelocity * (rand()%3+7)/10;
	else mBall.velocity.x = ballVelocity * (rand()%3+7)/10;
}


void testCollision(Brick& mBrick, Ball& mBall)
{
	
	if (!isIntersecting(mBrick, mBall)) return;

	mBrick.destroyed = true;

	score += 150;
	music.openFromFile("boink2.wav");
	music.setVolume(100);
	if(!stopmusic) music.play();
		
	float overlapLeft{ mBall.right() - mBrick.left() };
	float overlapRight{ mBrick.right() - mBall.left() };
	float overlapTop{ mBall.bottom() - mBrick.top() };
	float overlapBottom{ mBrick.bottom() - mBall.top() };
		
	bool ballfromLeft(abs(overlapLeft) < abs(overlapRight));
	bool ballfromTop(abs(overlapTop) < abs(overlapBottom));
	
	float minOverlapX{ ballfromLeft ? overlapLeft : overlapRight };
	float minOverlapY{ ballfromTop ? overlapTop : overlapBottom };

	if (abs(minOverlapX) < abs(minOverlapY)) 
	{
		mBall.velocity.x = ballfromLeft ? -ballVelocity : ballVelocity;
	}

	else
	{
		mBall.velocity.y = ballfromTop ? -ballVelocity : ballVelocity;
	}


}




class ParticleSystem1 : public sf::Drawable, public sf::Transformable
{
public:

	ParticleSystem1(unsigned int count) :
	m_particles(count),
	m_vertices(sf::Points, count),
	m_lifetime(sf::seconds(3.f)),
	m_emitter(0.f, 0.f)
	{
	}

	void setEmitter(sf::Vector2f position)
	{
		m_emitter = position;
	}

	void update(sf::Time elapsed)
	{
		for (std::size_t i = 0; i < m_particles.size(); ++i)
		{
			// update the particle lifetime
			Particle& p = m_particles[i];
			p.lifetime -= elapsed;

			// if the particle is dead, respawn it
			if (p.lifetime <= sf::Time::Zero)
				resetParticle(i);

			// update the position of the corresponding vertex
			m_vertices[i].position += p.velocity * elapsed.asSeconds();

			// update the alpha (transparency) of the particle according to its lifetime
			float ratio = p.lifetime.asSeconds() / m_lifetime.asSeconds();
			
			m_vertices[i].color.b = static_cast<sf::Uint8>(ratio * 255);
		}
	}

private:

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		// apply the transform
		states.transform *= getTransform();

		

		// our particles don't use a texture
		states.texture = NULL;

		// draw the vertex array
		target.draw(m_vertices, states);
	}

private:

	struct Particle
	{
		sf::Vector2f velocity;
		sf::Time lifetime;
	};

	void resetParticle(std::size_t index)
	{
		// give a random velocity and lifetime to the particle
		float angle = (std::rand() % 360) * 3.14f / 180.f;
		float speed = (std::rand() % 50) + 50.f;
		m_particles[index].velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
		m_particles[index].lifetime = sf::milliseconds((std::rand() % 500) );

		// reset the position of the corresponding vertex
		m_vertices[index].position = m_emitter;
	}

	std::vector<Particle> m_particles;
	sf::VertexArray m_vertices;
	sf::Time m_lifetime;
	sf::Vector2f m_emitter;
};


int main()
{
	sf::RenderWindow window(VideoMode(WindowWidth+250, WindowHeight), "Break The Bricks");
	window.setFramerateLimit(60);

	Ball ball{ WindowWidth / 2,WindowHeight / 2 };
	paddle Paddle{ WindowWidth / 2.f,WindowHeight - 5.f};

	vector <Brick> bricks;

	for (int iX{ 0 }; iX < countBlocksx; ++iX)
		for (int iY{ 0 }; iY < countBlocksy; ++iY)
			bricks.emplace_back((iX + 1) * (blockWidth + 3) + 22, (iY + 2) * (blockHeight + 3));

	Music game_music;
	
	game_music.openFromFile("SONG01.ogg");
	game_music.setVolume(35);
	Music gameover_music;
	
	if(!stopmusic) game_music.play();
	
	loading.loadFromFile("4.png");

	RectangleShape loadingscreen;
	
	Vector2f size1;
	size1.x = 100, size1.y = 100;
	loadingscreen.setTexture(&loading);
	loadingscreen.setFillColor(Color::White);
	loadingscreen.setScale({ 1,1 });
	
	Sprite gameover;
	Texture GMtexture;
	
	GMtexture.loadFromFile("gameover.png");
	gameover.setTexture(GMtexture);
	gameover.setPosition(WindowWidth/2.f -50, WindowHeight/2.f);
	gameover.setOrigin(202, 180);
	


	


	BallTexture.loadFromFile("ball.png");
	brickTexture.loadFromFile("goldbrick.png");
	texture.loadFromFile("score.png");
	font.loadFromFile("Impacted.ttf");
	Vector2u size = texture.getSize();
	sprite.setTexture(texture);
	ParticleSystem1 particles(200);
	
	sf::Clock clock;
	sf::Clock clock2;
	bool playscreen = false;

	Text h1,m1,m2,m3;
	h1.setFont(font);
	h1.setCharacterSize(100);
	h1.setPosition(250, 20);
	h1.setFillColor(Color::Black);
	h1.setString("BREAK THE BRICKS");
	h1.setRotation(2);
	h1.setOutlineThickness(5);
	h1.setOutlineColor(Color::Yellow);
	
	m1.setFont(font);
	m1.setCharacterSize(70);
	m1.setPosition(530, 300);
	m1.setFillColor(Color::Blue);
	m1.setString("PLAY");
	m1.setOutlineColor(Color::White);
	m1.setOutlineThickness(2);
	m1.Bold;

	m2.setFont(font);
	m2.setCharacterSize(70);
	m2.setPosition(530, 500);
	m2.setFillColor(Color::Blue);
	m2.setString("EXIT");
	m2.setOutlineColor(Color::White);
	m2.setOutlineThickness(2);
	m2.Bold;

	m3.setFont(font);
	m3.setCharacterSize(70);
	m3.setPosition(530, 400);
	m3.setFillColor(Color::Blue);
	m3.setString("MUSIC ON");
	m3.setOutlineColor(Color::White);
	m3.setOutlineThickness(2);
	m3.Bold;
	 
	while (window.isOpen()) 
	{
		Event Event;
		while (window.pollEvent(Event))
		{
			if (Event.type == Event::Closed)
				window.close();
		}
		
		
		FloatRect collision2 = m2.getGlobalBounds();
		FloatRect collision = m1.getGlobalBounds();
		FloatRect collision3 = h1.getGlobalBounds();
		FloatRect collision4 = m3.getGlobalBounds();
		
		Vector2f point;
		point.x = (float) Mouse::getPosition(window).x;
		point.y = (float) Mouse::getPosition(window).y;
		if (collision.contains(point))
		{
			m1.setFillColor(Color::Red);
			if (Event.type == Event.MouseButtonReleased && Event.mouseButton.button == Mouse::Left)
			{
				playscreen = true;
				break;
			}
		}
		else
		{
			m1.setFillColor(Color::Blue);
		}

		if (collision2.contains(point))
		{
			m2.setFillColor(Color::Red);
			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				playscreen = false;
				break;
			}
		}
		else
		{
			m2.setFillColor(Color::Blue);
		}

		if (collision3.contains(point))
		{
			h1.setFillColor(Color::Red);
			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				
			}
		}
		else
		{
			h1.setFillColor(Color::Black);
		}

		if (collision4.contains(point))
		{
			m3.setFillColor(Color::Red);
			if (Mouse::isButtonPressed(Mouse::Left))
			{
				
					m3.setString("MUSIC OFF");
					stopmusic = true;
					game_music.stop();
			}
				
			else if(Mouse::isButtonPressed(Mouse::Right))
			{
				m3.setString("MUSIC ON");
				stopmusic = false;
				game_music.play();
			}
		}
		else
		{
			m3.setFillColor(Color::Blue);
		}


		
		window.clear();
		window.draw(m2);
		window.draw(h1);
		window.draw(m3);
		window.draw(m1);
		window.display();
		if (Keyboard::isKeyPressed(Keyboard::Key::Enter))
		{
			playscreen = true;
			break;
		}

	}

	while (playscreen)
	{
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();
		}
		if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		

		if(!isGameOver)
		{
			if (!game_music.Playing && !stopmusic) game_music.play();
			 game_music.setLoop(true);
			 window.clear(Color::Black);
			 gameover_music.openFromFile("Paddlebreak.wav");
			 gameover_music.setVolume(210);
			 
			 sf::Vector2i mouse;
			 mouse.x = (int)ball.shape.getPosition().x;
			 mouse.y = (int)ball.shape.getPosition().y;
			 particles.setEmitter(window.mapPixelToCoords(mouse));
			 
			 loadingscreen.setSize(size1);
			 loadingscreen.setPosition(1070, 190);
			 loadingscreen.setOrigin({ size1.x / 2, size1.y / 2 });
			 testCollision(Paddle, ball);
			 paddleTexture.loadFromFile("paddle.png");
			 
			 points.setFont(font);
			 points.setStyle(Text::Bold);
			 points.StrikeThrough;
			 points.setOutlineColor(Color::Black);
			 points.setCharacterSize(60);
			 points.setOutlineThickness(10);
			 points.setPosition(1015, 460);

			 lvltxt.setFont(font);
			 lvltxt.setStyle(Text::Bold);
			 lvltxt.setOutlineColor(Color::Black);
			 lvltxt.setCharacterSize(75);
			 lvltxt.setOutlineThickness(6);
			 lvltxt.setPosition(1133, 258);
			 
			 std::stringstream ss;
			 std::stringstream ss2;
			 ss << score;
			 ss2 << level;
			 points.setString(ss.str());
			 lvltxt.setString(ss2.str());
			 
			 ball.update();
			 Paddle.update();
			 
			 
			 sf::Time elapsed = clock.restart();
			 particles.update(elapsed);
			 
			 loadingscreen.rotate(4);
			 
			 Paddle.shape.setTexture(&paddleTexture);
			 ball.shape.setTexture(&BallTexture);
			 sprite.setPosition(WindowWidth, 0);
			 
			 
			 
			 for (auto& brick : bricks) testCollision(brick, ball);

			 bricks.erase(remove_if(begin(bricks), end(bricks), [](const Brick& mBrick) { return mBrick.destroyed; }),
			 	end(bricks));
			 
			 for (auto& brick : bricks) brick.shape.setTexture(&brickTexture);
			 
			 
			 
			 window.draw(ball.shape);
			 window.draw(Paddle.shape);
			 for (auto& brick : bricks) window.draw(brick.shape);
			 window.draw(sprite);
			 window.draw(points);
			 window.draw(lvltxt);
			 window.draw(loadingscreen);
			 window.draw(particles);
			 window.display();
			 if(isGameOver && !stopmusic) 
			 {
				 gameover_music.play();
			 }
		}
		else
		{
			game_music.stop();
			
			
			//gameover.rotate(1);
			window.draw(gameover);
			window.display();
			if (Keyboard::isKeyPressed(Keyboard::Key::Enter))
			{
				bricks.erase(remove_if(begin(bricks), end(bricks), [](const Brick& mBrick) { return !mBrick.destroyed; }),
					end(bricks));
				
				isGameOver = false;
				ball.shape.setPosition({(rand()%WindowWidth)/1.f,WindowHeight-100});
				ball.update();
				ball.velocity.y = -ballVelocity;
				score = 0;
				for (int iX{ 0 }; iX < countBlocksx; ++iX)
					for (int iY{ 0 }; iY < countBlocksy; ++iY)
						bricks.emplace_back((iX + 1) * (blockWidth + 3) + 22, (iY + 2) * (blockHeight + 3));
			}
		}
	}

	

	return 0;


}