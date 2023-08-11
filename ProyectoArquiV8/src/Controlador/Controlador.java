/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Controlador;

import javax.swing.ImageIcon;
import javax.swing.JFrame;


/**
 *
 * @author Alex Altuve
 */
public class Controlador {
   
    
    public void iniciaVentana(JFrame ventana, String ruta) {
        ventana.setLocationRelativeTo(null);
        ventana.setIconImage(new ImageIcon(ruta).getImage());
        ventana.setResizable(false);
    }
    public void activaVentana(JFrame ventana,JFrame ventana2) {
        
        ventana.setLocationRelativeTo(null);
        ventana.setVisible(true);
        ventana2.dispose(); 
    }
    
}
