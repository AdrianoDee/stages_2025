#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;


//Structure repr�sentant un point en 2D
struct P {
    double x;
    double y;
};

//choix seuil de distance
double dc = 0.953;       //0.953, 0.921, 0.777 0.41, 0.07

//Fonct� qui calcule la dist euclid entre deux points
double EuDist(const P& a, const P& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
};


int main() {
    ifstream f("../simple_data/R1_data.csv");
    if (!f.is_open()) {
        cerr << "Erreur : impossible d'ouvrir le fichier." << endl;
        return 1;
    }

    vector<P> points; //D�claraton, car utile apr�s
    string line;

    //Ignorer l'en-t�te
    getline(f, line);

    //Lire les lignes du fichier
    while (getline(f, line)) {
        stringstream ss(line);
        string x_str, y_str, label_str;

        getline(ss, x_str, ',');
        getline(ss, y_str, ',');
        getline(ss, label_str); //Ignor�

        if (!x_str.empty() && !y_str.empty()) {
            double x = stod(x_str);
            double y = stod(y_str);
            points.push_back({x, y});
        }
    }

    f.close();

//Remplissage de la matrcie
    size_t NPtot = points.size();
    vector<vector<double>> MatDij(NPtot, vector<double>(NPtot, 0.0));

  for (size_t i = 0; i < NPtot; ++i) {
    for (size_t j = i + 1; j < NPtot; ++j) {
        double dist = EuDist(points[i], points[j]);
        MatDij[i][j] = dist;
        MatDij[j][i] = dist;
    }

}

//Calcul des rho_i pour chaque point
vector<double> rho(NPtot, 0.0);     //on d�clare un vecteur, contenant NPtot point, tous initialis� � 0

for (size_t i = 0; i<NPtot; ++i){
        for (size_t j = 0; j<NPtot; ++j){
                if (i != j && MatDij[i][j] < dc){    // != i diff de j, && l'un est l'autre c�d i diff j et MatDij < dc
                    rho[i] += 1.0;
                }
        }
}

//Structure du delta i
vector<double> delta(NPtot, 0.0);
vector<int> proche_superieur(NPtot, -1); //Si c'est le point de plus haute densit�, renvoye -1 = juste une valeur diff�rente pour qu'on sache la rep�rer

//Trouvaille densit� max
size_t rho_max = 0; //Initialise avec le premier �l�ment, c�d indice 0 � rho_max.
for (size_t i = 1; i < NPtot; ++i){ //Puis, parcour depuis l'indice 1 en essayant de trouver un meilleur candidat
    if(rho[i] > rho[rho_max]){
            rho_max = i;
    }
}

//Trouvaille distance minimale
for (size_t i = 0; i < NPtot; ++i){
    double min_dist = numeric_limits<double>::max(); //La + grande valeur que possible, et si on trouves une val minimal � �a, on lui affecte
    for (size_t j = 0; j < NPtot; ++j){
        if (rho[j] > rho[i] && MatDij[i][j] < min_dist){
        min_dist = MatDij[i][j];
        proche_superieur[i] = j;
        }
    }


    if (proche_superieur[i] != -1){
    delta[i] = min_dist;
    }

//Et pour le point de plus grande densit�,on prend max_dist :
    else{
        double max_dist = 0.0;
        for (size_t j = 0; j < NPtot; ++j){
            if (i != j && MatDij[i][j] > max_dist){
            max_dist = MatDij[i][j];
            }
        }
    delta[i] = max_dist;
    }
}

//Calcul des gamma i
vector<double> gamma (NPtot, 0.0);
for (size_t i = 0; i<NPtot; ++i){
gamma[i] = rho[i] * delta[i];
}


//Affichage
/*
   cout << fixed << setprecision(2);
    cout << "Matrice des distances d_ij :" "\n\n";
    for (size_t i = 0; i < NPtot; ++i) {
        for (size_t j = 0; j < NPtot; ++j) {
            cout << MatDij[i][j] << "\t";
        }
        cout << "\n\n";
    }

cout << "Densites locales (rho_i) :" "\n\n";
for (size_t i = 0; i<NPtot; ++i){
        cout << "rho [" << i << "] =" << rho[i] << endl;

    }
*/
cout << endl; //cr�er un espace dans l'affichage entre le dernier rho et la phrase "Distance plus..."

cout << "Distance plus proches % point de densite superieur (delta_i) :" "\n\n";

size_t spe_delta = 0;

for (size_t i = 0; i < NPtot; ++i){
    cout << "delta[" << i << "] = " << delta[i] ;
    if (proche_superieur[i] != -1){
        cout << " point le + proche avec une densite superieure, point Num : " << proche_superieur[i] ;
    }
        else{
            cout << "\033[1;33m" << " point de + haute densite ayant pour valeur : " << rho[i] << "\033[0m"; // \033[ = d�but s�quence couleur, \033[0m = r�initialise les couleurs et 1;31m = rouge vif (1;33m jaune, 1;34m bleu)
        spe_delta ++;
        }
    cout << endl;
}

cout << "\nnbre de delta max trouve : " << spe_delta << ", avec pour valeur de dc = " << dc << endl;

cout << "\n\nGamma(i) : \n";
for (size_t i = 0; i < NPtot; ++i){
    cout << "gamma [" << i << "] = " << gamma[i] << endl;
}



// Exporter les gamma_i dans un fichier CSV
ofstream fout("gamma_output.csv");
if (!fout.is_open()) {
    cerr << "Erreur : impossible de cr�er le fichier gamma_output.csv" << endl;
    return 1;
}

fout << "index,gamma_i\n";  // En-t�te

for (size_t i = 0; i < NPtot; ++i) {
    fout << i << "," << gamma[i] << "\n";
}

fout.close();




//Affectation des points � un cluster
//Initialisation : -1 = non affect�
vector<int> cluster_assignments(NPtot, -1);
int cluster_id = 1;

//�tape 1 : Identifier les centres (points sans sup�rieur de densit�)
for (size_t i = 0; i < NPtot; ++i) {
    if (proche_superieur[i] == -1) {
        cluster_assignments[i] = cluster_id;
        cluster_id++;
    }
}

// �tape 2 : Affecter les autres points au m�me cluster que leur plus proche point de densit� sup�rieure
// On parcourt les points par ordre d�croissant de densit� (rho), pour que les plus denses soient affect�s d'abord

vector<int> indices(NPtot);
for (size_t i = 0; i < NPtot; ++i) indices[i] = i;

// Trier les indices par ordre d�croissant de rho
sort(indices.begin(), indices.end(), [&](int a, int b) {   //[&]= capturer variable ext�rieur par r�f�rence. Donc sinon ne conna�trais pas les rhoalors que la, peut y acc�der
    return rho[a] > rho[b];
});

for (size_t i = 0; i < NPtot; ++i) {
    int idx = indices[i];
    if (cluster_assignments[idx] == -1) {
        cluster_assignments[idx] = cluster_assignments[proche_superieur[idx]];
    }
}

//exporter en .csv
ofstream fclusters("cluster_assignments_0.953.csv");
if (!fclusters.is_open()) {
    cerr << "Erreur : impossible de cr�er le fichier cluster_assignments.csv" << endl;
    return 1;
}

fclusters << "x,y,cluster\n";
for (size_t i = 0; i < NPtot; ++i) {
    fclusters << points[i].x << "," << points[i].y << "," << cluster_assignments[i] << "\n";
}


fclusters.close();

cout << "\nAffectation des clusters faite. Exportee dans 'cluster_assignments.csv'.\n";






    return 0;
}
