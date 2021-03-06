
import java.awt.BorderLayout;
import java.awt.Canvas;
import java.awt.Color;
import java.awt.Graphics;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.LineNumberReader;


import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;



public class DisplayECG extends JFrame implements Runnable,ActionListener {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	private static boolean AC_MODE = true;
	
	private static final double scale = 100000;
	private int offset = 0;
	
	private Canvas canvas;
	private JLabel currentValue;
	JButton fadeButton;
	LineNumberReader lnr;
	private boolean fadeEnable=true;
	
	public static void main(String args[]) throws IOException {
		DisplayECG osc = new DisplayECG();
		
		if ("-".equals(args[0])) {
			osc.lnr = new LineNumberReader(new InputStreamReader(System.in));
		} else {
			File ioDevice = new File(args[0]);
			FileReader r = new FileReader(ioDevice);
			osc.lnr = new LineNumberReader(r);
		}
	}
	public DisplayECG () {
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setLayout(new FlowLayout());
		this.setSize(1024, 980);
		
		JPanel p = new JPanel(new FlowLayout());
		p.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createTitledBorder("blah"),
                BorderFactory.createEmptyBorder(5,5,5,5)));
		add(p);
		currentValue = new JLabel("");
		//currentValue.setBackground(Color.YELLOW);
		
		p.add(currentValue);
		
		JButton clrBtn = new JButton("CLR");
		clrBtn.setActionCommand("clear");
		clrBtn.addActionListener(this);
		
		p.add(clrBtn);
		p.add(new JButton("GRID"));
		p.add(new JButton("+"));
		p.add(new JButton("-"));
		
		fadeButton = new JButton("FADE");
		fadeButton.setActionCommand("fade");
		fadeButton.addActionListener( this);
		p.add(fadeButton);
		
		canvas = new Canvas();
		canvas.setSize(1000, 950);
		add(canvas);
				
	
		//canvas.getGraphics().drawRect(0,0,100,100);
		
		setVisible(true);
		
		Thread runThread = new Thread(this);
		runThread.start();
	}


	public void run() {
		
		Graphics g = canvas.getGraphics();
		int x = 0, y,xm1;
		int width = canvas.getWidth();
		int height = canvas.getHeight();
		int yzero = height/2;

		g.setColor(Color.black);
		g.drawLine (0,0,width,0);
		g.drawLine (0,yzero,width,yzero);

		g.setColor(Color.green);

		double v;
		while (true) {
			v = getADC();
			y = yzero-(int)(v*scale);
			g.drawRect( x, y,2,2);
			x++;
			if (x >= width) {
				x = 0;
			}
		}
		
	}
	private double getADC ()  {
		try {
			String line = lnr.readLine();
			String[] p = line.split("\\s");
			return Double.parseDouble(p[1]);
		} catch (Exception e) {
			return -1;
		}
	}
	public void actionPerformed(ActionEvent e) {
		String cmd = e.getActionCommand();
		System.err.println ("cmd=" + cmd);
		if ("clear".equals(cmd)) {
			canvas.getGraphics().clearRect(0, 0, canvas.getWidth(), canvas.getHeight());
		}
	}
}
