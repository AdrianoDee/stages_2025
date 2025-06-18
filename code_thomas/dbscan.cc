#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>


class DBSCAN
{
public:
    //On créé un tableau pour x et y qui sont des nombres à virgule
    struct Point
    {
        double x;
        double y;
    };


private:
    std::vector<Point> points;
    std::vector<int> label;
    std::vector<std::string> type_point;
    int Minpts;
    double epsilon;


    //Fonction pour calculer la distance euclidienne entre deux points
    double dist(const Point a, const Point b) //on associe a et b de la forme du Point créé précédemment (avec 2 coordonées)
    {
        return std::sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)); //"." prend la valeur x et y du point a ou b
    }



    //Fonction pour trouver les voisins d'un point et leur position (distance inférieure à epsilon)
    std::vector<int> Voisinage(int indice)
    {
        std::vector<int> voisins; //on initialise un vecteur vide pour trouver les indices des voisins trouvés
        const Point p=points[indice]; //le point p est le point dont on cherche les voisins

        for (int i=0; i<points.size(); ++i) //on parcours tous les points
        {
            if (i==indice) continue; //pour ne pas calculer sa position par rapport à sois-même
            if(dist(p, points[i]) <= epsilon) //calcul de la distance de tous les points[i] par rapport à p
            {
                voisins.push_back(i); //pour ajouter l'indice du voisin au vecteur voisins
            }
        }
        return voisins;
    }




public:
    DBSCAN(const std::vector<Point>& pts, double eps, int minpts):
         points(pts), epsilon(eps), Minpts(minpts), label(pts.size(),0), type_point(pts.size()){}

    void run()
    {
        int NumCluster=1; //chaque fois qu'on aura un nouveau cluster, on augmente de 1
        /*
        label[i]=0 --> le point n'a pas été visité
        label[i]=-1 --> bruit
        label[i]=1,2,.. --> le point appartient au cluster 1,2,..
        */
        for (int i=0; i<points.size(); ++i) //on parcours tous les points de la liste
        {
            if (label[i]!=0) continue; //on s'assure qu'un point n'est visité qu'une fois

            std::vector<int> voisins = Voisinage(i); //on cherche les voisins de points[i] (< epsilon)

            if(voisins.size()+1<Minpts)
            {
                label[i]= -1; //bruit
                continue;
            }
            else
            {
                label[i]= NumCluster; //si ce n'est pas du bruit alors ce point appartient à un cluster
                std::vector<int> Propagation_voisins = voisins; //on commence à propager le cluster
                for (int j=0; j < Propagation_voisins.size(); ++j) //on parcours maintenant les voisins du core point trouvé juste avant
                {
                    int k=Propagation_voisins[j]; //on prend le j-ième voisin de la liste et son indice est k
                    if (label[k]==-1)
                    {
                        label[k]=NumCluster; //s'il avait été marqué comme bruit mais est finaement proche d'un core point, on le réintègre au nouveau cluster
                    }
                    if(label[k]!=0) continue; //fait déjà partie d'un cluster, on l'ignore
                    {
                        label[k]=NumCluster;
                    }
                    std::vector<int> voisins_de_voisin= Voisinage(k);
                    if (voisins_de_voisin.size()+1 >= Minpts) //si le voisin est lui-même core point, alors on explore ses voisins
                    {
                        for (int idx: voisins_de_voisin)
                        if (label[idx]==0)
                        {
                            Propagation_voisins.push_back(idx);
                            label[idx]=NumCluster;
                        }

                    }
                }

                NumCluster++;
            }
        }


        for(int i=0; i<points.size(); ++i)
        {
            if (label[i]== -1)
            {
                type_point[i]="Noise point";
            }
            else
            {
                // on recalcule les voisins pour savoir si c'est un Core ou Border
                std::vector<int> voisins=Voisinage(i);
                if (voisins.size()+1>= Minpts)
                {
                    type_point[i]="Core point";
                }
                else
                {
                    type_point[i]="Border point";
                }
            }
            std::cout<<"Point numero "<<i<<" dans le Cluster "<<label[i]<<" ,Type: "<<type_point[i]<<std::endl;
        }
            //Calcul et écriture des centres des clusters
        int nbClusters= NumCluster - 1;   // car NumCluster a été incrémenté après le dernier cluster

        if (nbClusters>0)
        {
            std::vector<double> sum_x(nbClusters + 1, 0.0);
            std::vector<double> sum_y(nbClusters + 1, 0.0);
            std::vector<int>    compte(nbClusters + 1, 0);

            for (int i=0; i<points.size(); ++i)
            {
                int c=label[i];
                if (c>0)
                {
                    sum_x[c]+= points[i].x;
                    sum_y[c]+= points[i].y;
                    compte[c]+= 1;
                }
            }

            std::ofstream fcentres("Centres_DBSCAN.csv");
            fcentres << "cluster,centre_x,centre_y\n";

            std::cout << "Centres des clusters ---\n";
            for (int c=1; c<=nbClusters; ++c)
            {
                if (compte[c]==0) continue;
                double cx=sum_x[c]/compte[c];
                double cy=sum_y[c]/compte[c];
                std::cout << "Cluster " << c << "  centre = (" << cx << ", " << cy << ")\n";
                fcentres << c << ',' << cx << ',' << cy << '\n';
            }

            fcentres.close();
        }

    }

    //Pour le plot avec python
    void fichier_csv(const std::string& fichier_csv)
    {
        std::ofstream fout("Plot.csv");
        fout << "x,y,cluster,type\n";
        for (int i = 0; i < points.size(); ++i)
        {
            fout << points[i].x << "," << points[i].y << "," << label[i] << "," << type_point[i] << "\n";
        }
        fout.close();
    }

};









int main()
{
    std::ifstream f;
    f.open("tracks_z.csv"); //ouverture du fichier .csv
    std::string line; // on créé une variable ligne qui contiendra chaque ligne du fichier
    std::getline(f, line); //getline avant la boucle permet d'ignorer la première ligne

    std::vector<DBSCAN::Point> points;
    while (std::getline(f, line))
    {
        std::stringstream s(line); //pour pouvoir lire x et y séparément sur une même ligne
        double z,pt;
        char virgule;
        s>>z>>virgule>>pt;
        //if (pt<2) continue;
        //if (std::abs(z) > 10.0) continue;
        points.push_back({z,0.0});//ajoute le point p au vecteur points
    }
    f.close();



    int Minpts=4;
    double epsilon;
    std::cout<<"Choisir epsilon= ";
    std::cin>>epsilon;

    DBSCAN code(points, epsilon, Minpts);
    code.run();
    code.fichier_csv("Plot.csv");


    return 0;
}
