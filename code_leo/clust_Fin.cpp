#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <algorithm>
#include <set>
#include <numeric>
#include <functional>

using namespace std;

struct P {
    double x, y;
};

//Fonct° qui calcule la dist euclid entre deux points
double EuDist(const P& a, const P& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
};

int main() {
    //Lecture des points depuis le fichier CSV
    ifstream f("R1_data.csv");
    if (!f.is_open()) {
        cerr << "Erreur : impossible d'ouvrir le fichier." << endl;
        return 1;
    }

    vector<P> points;
    string line;
    getline(f, line); //ignorer l’en-tête

    while (getline(f, line)) {
        stringstream ss(line);
        string x_str, y_str, label_str;
        getline(ss, x_str, ',');
        getline(ss, y_str, ',');
        getline(ss, label_str); //on ignore le label

        if (!x_str.empty() && !y_str.empty()) {
            points.push_back({stod(x_str), stod(y_str)});
        }
    }
    f.close();

    size_t NPtot = points.size();


    //Calcul de la matrice des distances
    vector<vector<double>> MatDij(NPtot, vector<double>(NPtot));
    for (size_t i = 0; i < NPtot; ++i)
        for (size_t j = i + 1; j < NPtot; ++j) {
            double dist = EuDist(points[i], points[j]);
            MatDij[i][j] = dist;
            MatDij[j][i] = dist;
        }

    //Paramètres utilisateur
    double dc, seuil_gamma;
    cout << "Entrez la valeur de dc : ";
    cin >> dc;
    cout << "Entrez le seuil sur gamma (ex: 40) : ";
    cin >> seuil_gamma;
    double min_separation = 2 * dc;

    //Calcul de rho
    vector<double> rho(NPtot, 0.0);
    for (size_t i = 0; i < NPtot; ++i)
        for (size_t j = 0; j < NPtot; ++j)
            if (i != j && MatDij[i][j] < dc)
                rho[i] += 1.0;

    //Calcul de delta et proche supérieur
    vector<double> delta(NPtot, 0.0);
    vector<int> proche_superieur(NPtot, -1);
    for (size_t i = 0; i < NPtot; ++i) {
        double min_dist = numeric_limits<double>::max();
        for (size_t j = 0; j < NPtot; ++j) {
            if ((rho[j] > rho[i] || (rho[j] == rho[i] && j < i)) && MatDij[i][j] < min_dist){ //Putain de bordel : j'ignorait le cas où rho[j]==rho[i] donc certains point ne trouvait jamais de proche supérieur, d'où affectat° hasardeuse car pour tout les points rhoj==rhoi bah la cond° échouait
                min_dist = MatDij[i][j];
                proche_superieur[i] = j;
            }
        }
        delta[i] = (proche_superieur[i] != -1)
                 ? MatDij[i][proche_superieur[i]]
                 : *max_element(MatDij[i].begin(), MatDij[i].end());
    }

    //Calcul de gamma
    vector<double> gamma(NPtot);
    for (size_t i = 0; i < NPtot; ++i)
        gamma[i] = rho[i] * delta[i];

    //Tri des indices par gamma décroissant
    vector<int> indices(NPtot);
    iota(indices.begin(), indices.end(), 0);
    sort(indices.begin(), indices.end(), [&](int a, int b) { return gamma[a] > gamma[b]; });

    //Sélection des indices avec gamma > seuil_gamma
    vector<int> candidats;
    for (int idx : indices) {
        if (gamma[idx] > seuil_gamma) {
            candidats.push_back(idx);
        }
    }

    cout << "Nombre de points avec gamma > seuil : " << candidats.size() << endl;

    //Filtrage parmi les candidats en imposant une distance minimale de 2*dc
    vector<int> centre_indices;
    for (int idx : candidats) {
        bool trop_proche = false;
        for (int c : centre_indices) {
            if (MatDij[idx][c] < min_separation) {
                trop_proche = true;
                break;
            }
        }
        if (!trop_proche) {
            centre_indices.push_back(idx);
        }
    }

    //Affichage des centres trouvés
    cout << "Nombre de centres trouves : " << centre_indices.size() << endl;
    for (int idx : centre_indices)
        cout << "Centre " << idx << " (x=" << points[idx].x << ", y=" << points[idx].y << ", gamma=" << gamma[idx] << ")\n";

    if (centre_indices.empty()) {
        cerr << "Aucun centre detecte au-dessus du seuil gamma avec séparation >= 2*dc.\n";
        return 1;
    }

    //Export des centres dans un fichier
    ofstream fcentres("centres.csv");
    if (!fcentres.is_open()) {
        cerr << "Erreur lors de l'ouverture du fichier des centres.\n";
        return 1;
    }
    fcentres << "index\n";
    for (int idx : centre_indices)
        fcentres << idx << "\n";
    fcentres.close();

 //Affectation des clusters
vector<int> cluster_assignments(NPtot, -1);

//Étape 1 : assigner chaque centre à un cluster unique
for (size_t i = 0; i < centre_indices.size(); ++i) {
    int idx = centre_indices[i];
    cluster_assignments[idx] = i + 1; //Clusters indexés à partir de 1
}

//Étape 2 : trier les points par densité décroissante
vector<int> sorted_rho_indices(NPtot);
iota(sorted_rho_indices.begin(), sorted_rho_indices.end(), 0);
sort(sorted_rho_indices.begin(), sorted_rho_indices.end(),
     [&](int a, int b) { return rho[a] > rho[b]; });

//Fonction récursive avec fallback pour points sans proche_superieur
 function<int(int)> assign_cluster = [&](int i) -> int {
    if (cluster_assignments[i] != -1)         //Faux : cluster_assignments[i] = cluster_assignments[proche_superieur[i]] car si ps n'a pas été affecté ? Donc ps =-1 donc risque de propager un clust non défini
        return cluster_assignments[i];
    if (proche_superieur[i] == -1)
        return -1; // Aucun centre atteignable

    int parent_cluster = assign_cluster(proche_superieur[i]);
    cluster_assignments[i] = parent_cluster;
    return parent_cluster;
};

for (int i : sorted_rho_indices) {
    assign_cluster(i);
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

cout << endl; //créer un espace dans l'affichage entre le dernier rho et la phrase "Distance plus..."

cout << "Distance plus proches % point de densite superieur (delta_i) :" "\n\n";

size_t spe_delta = 0;

for (size_t i = 0; i < NPtot; ++i){
    cout << "delta[" << i << "] = " << delta[i] ;
    if (proche_superieur[i] != -1){
        cout << " point le + proche avec une densite superieure, point Num : " << proche_superieur[i] ;
    }
        else{
            cout << "\033[1;33m" << " point de + haute densite ayant pour valeur : " << rho[i] << "\033[0m"; // \033[ = début séquence couleur, \033[0m = réinitialise les couleurs et 1;31m = rouge vif (1;33m jaune, 1;34m bleu)
        spe_delta ++;
        }
    cout << endl;
}

*/













    // Export CSV final
    ofstream fout("cluster_fin.csv");
    if (!fout.is_open()) {
        cerr << "Erreur lors de l'ouverture du fichier de sortie.\n";
        return 1;
    }
    fout << "x,y,cluster\n";
    for (size_t i = 0; i < NPtot; ++i)
        fout << points[i].x << "," << points[i].y << "," << cluster_assignments[i] << "\n";

    fout.close();
    cout << "Export termine dans 'cluster_fin.csv'.\n";

    return 0;
}
