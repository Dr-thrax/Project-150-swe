#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;

constexpr float ballRadius{ 10.0f }, ballVelocity{ 8.f };
constexpr int WindowHeight{ 720 }, WindowWidth{ 1250 };
 float paddleWidth{ 120.0f }, paddleHeight{ 10.0f }, paddleVelocity{16.f};
constexpr float blockWidth{ 60.f }, blockHeight{ 20.f };
constexpr int countBlocksx{ 18 }, countBlocksy{ 4 };

Music music;
Music music2;

Texture paddleTexture;
Texture BallTexture;
Texture brickTexture;

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

	}

	void update() 
	{
		shape.move(velocity);

		if (left() < 0) velocity.x = ballVelocity;
		else if (right() > WindowWidth) velocity.x = -ballVelocity;
		if (top() < 0) velocity.y = ballVelocity;
		else if (bottom() > WindowHeight) velocity.y = -ballVelocity;

		
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
		shape.setFillColor(Color::Green);
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
	music2.play();

	if (mBall.x() < mpaddle.x()) mBall.velocity.x = -ballVelocity;
	else mBall.velocity.x = ballVelocity;
}


void testCollision(Brick& mBrick, Ball& mBall)
{
	if (!isIntersecting(mBrick, mBall)) return;

	mBrick.destroyed = true;
	music.openFromFile("boink2.wav");
	music.setVolume(100);
	music.play();
		
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
int main()
{
	sf::RenderWindow window(VideoMode(WindowWidth, WindowHeight), "Break The Bricks");
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
	game_music.play();


	BallTexture.loadFromFile("ball.png");
	brickTexture.loadFromFile("goldbrick.png");

	while (true)
	{
		 
		game_music.setLoop(true);
		window.clear(Color::Black);
		
		if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

		ball.update();
		Paddle.update();
		testCollision(Paddle, ball);
		paddleTexture.loadFromFile("paddle.png");
		Paddle.shape.setTexture(&paddleTexture);
		ball.shape.setTexture(&BallTexture);
		

		for (auto& brick : bricks) testCollision(brick, ball);
		
		bricks.erase(remove_if(begin(bricks), end(bricks), [](const Brick& mBrick) { return mBrick.destroyed; }),
			end(bricks));

		for (auto& brick : bricks) brick.shape.setTexture(&brickTexture);

		window.draw(ball.shape);
		window.draw(Paddle.shape);
		for (auto& brick : bricks) window.draw(brick.shape);
		window.display();
	}

	return 0;


}