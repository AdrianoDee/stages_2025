#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>


class DBSCAN
{
public:
    //On crÈÈ un tableau pour x et y qui sont des nombres ‡ virgule
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
    double dist(const Point a, const Point b) //on associe a et b de la forme du Point crÈÈ prÈcÈdemment (avec 2 coordonÈes)
    {
        return std::sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y)); //"." prend la valeur x et y du point a ou b
    }



    //Fonction pour trouver les voisins d'un point et leur position (distance infÈrieure ‡ epsilon)
    std::vector<int> Voisinage(int indice)
    {
        std::vector<int> voisins; //on initialise un vecteur vide pour trouver les indices des voisins trouvÈs
        const Point p=points[indice]; //le point p est le point dont on cherche les voisins

        for (int i=0; i<points.size(); ++i) //on parcours tous les points
        {
            if (i==indice) continue; //pour ne pas calculer sa position par rapport ‡ sois-mÍme
            if(dist(p, points[i]) <= epsilon) //calcul de la distance de tous les points[i] par rapport ‡ p
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
        label[i]=0 --> le point n'a pas ÈtÈ visitÈ
        label[i]=-1 --> bruit
        label[i]=1,2,.. --> le point appartient au cluster 1,2,..
        */
        for (int i=0; i<points.size(); ++i) //on parcours tous les points de la liste
        {
            if (label[i]!=0) continue; //on s'assure qu'un point n'est visitÈ qu'une fois

            std::vector<int> voisins = Voisinage(i); //on cherche les voisins de points[i] (< epsilon)

            if(voisins.size()<Minpts)
            {
                label[i]= -1; //bruit
            }
            else
            {
                label[i]= NumCluster; //si ce n'est pas du bruit alors ce point appartient ‡ un cluster
                std::vector<int> Propagation_voisins = voisins; //on commence ‡ propager le cluster
                for (int j=0; j < Propagation_voisins.size(); ++j) //on parcours maintenant les voisins du core point trouvÈ juste avant
                {
                    int k=Propagation_voisins[j]; //on prend le j-iËme voisin de la liste et son indice est k
                    if (label[k]==-1)
                    {
                        label[k]=NumCluster; //s'il avait ÈtÈ marquÈ comme bruit mais est finaement proche d'un core point, on le rÈintËgre au nouveau cluster
                    }
                    if(label[k]!=0) continue; //fait dÈj‡ partie d'un cluster, on l'ignore
                    {
                        label[k]=NumCluster;
                    }
                    std::vector<int> voisins_de_voisin= Voisinage(k);
                    if (voisins_de_voisin.size() >= Minpts) //si le voisin est lui-mÍme core point, alors on explore ses voisins
                    {
                        Propagation_voisins.insert(Propagation_voisins.end(), voisins_de_voisin.begin(),voisins_de_voisin.end()); //On ajoute tous les voisins de k ‡ la liste des points ‡ explorer ce qui Ètend le cluster
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
                if (voisins.size()>= Minpts)
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
    f.open("../simple_data/D1_data.csv"); //ouverture du fichier .csv
    std::string line; // on crÈÈ une variable ligne qui contiendra chaque ligne du fichier
    std::getline(f, line); //getline avant la boucle permet d'ignorer la premiËre ligne

    std::vector<DBSCAN::Point> points;
    while (std::getline(f, line))
    {
        std::stringstream s(line); //pour pouvoir lire x et y sÈparÈment sur une mÍme ligne
        double x,y;
        char virgule1;
        s>>x>>virgule1>>y;
        points.push_back({x,y});//ajoute le point p au vecteur points
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
