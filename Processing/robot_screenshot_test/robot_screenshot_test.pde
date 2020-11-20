import java.awt.*;
import java.awt.image.*;

Robot robot;
Rectangle bounds;

void setup() {
  String p = System.getProperty("java.version") + " | " +
  System.getProperty("java.home") + " | " +
  System.getProperty("java.vendor") + " | " +
  System.getProperty("java.vendor.url");
  println(p);

  size(1280, 720);

  try {
    robot = new Robot();
    bounds = new Rectangle(1920, 1080);
  } catch (AWTException e) {
    println("can't initialize robot");
  }
}

void draw() {
  if (robot == null) {
    return;
  }
  BufferedImage screenshot = robot.createScreenCapture(bounds);
  image(new PImage(screenshot), 0, 0, width, height);
}
