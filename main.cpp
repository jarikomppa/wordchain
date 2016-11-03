#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unordered_map>

#define MAXTOKENS 8192
#define MAXNEXTTOKENS 1024

struct prevkeys
{
	int a, b, c, d;
	prevkeys()
	{
		reset();
	}
	void reset()
	{
		a = b = c = d = -1;
	}
	void shift()
	{
		d = c;
		c = b;
		b = a;
		a = -1;
	}
	bool operator==(const prevkeys &other) const
	{
		return (a == other.a) && (b == other.b) && (c == other.c) && (d == other.d);
	}
};

namespace std {

	template <>
	struct hash<prevkeys>
	{
		std::size_t operator()(const prevkeys& k) const
		{
			return (k.a << 1)^(k.b << 3)^(k.c << 2)^(k.d);
		}
	};

}

int whitespace(char aCharacter)
{
	switch (aCharacter)
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
		return 1;
	}
	return 0;
}

int is_numeric(char *aString)
{
	if (!*aString) return 0;
	while (*aString)
	{
		if (*aString < '0' || *aString > '9')
			return 0;
		aString++;
	}
	return 1;
}

int is_alpha(char *aChar)
{
	if (!*aChar) return 0;
	if (toupper(*aChar) < 'A' || toupper(*aChar) > 'Z')
		return 0;
	return 1;
}


class WordCounter
{
public:
	std::unordered_map<int, char*> mWord;
	std::unordered_map<int, int> mFlags, mHits, mHash, mValid;
	std::unordered_map<int, std::unordered_map<int, int>> mNext, mNextHit;
	std::unordered_map<prevkeys, std::unordered_map<int, int>> mLongHit;
	prevkeys mPrevKeys;
	int mPrevToken;
	int mTokens;
    int mWordno;	

	int calcHash(char *aString)
	{
		unsigned int i = 0;
		while (*aString)
		{
			i = (i << 11) | (i >> 21);
			i ^= *aString;
	        aString++;
		}    
    
		return i;
	}    

	WordCounter()
	{
		mPrevToken = -1;
		mTokens = 0;
		mWordno = 0;		
	}

	void tokenRef(int aCurrent)
	{
		mLongHit[mPrevKeys][aCurrent]++;

		mPrevKeys.shift();
		mPrevKeys.a = aCurrent;

		if (mWordno == 0)
			mPrevKeys.reset();



		int prev = mPrevToken;
		mPrevToken = aCurrent;
		if (prev != -1)
		{
			int i;
			int s = (signed)mNext[prev].size();
			for (i = 0; i < s; i++)
			{
				if (mNext[prev][i] == aCurrent)
				{
					mNextHit[prev][i]++;
					return;
				}
			}
			if (s < MAXNEXTTOKENS)
			{
				int i = s;
				mNext[prev][i] = aCurrent;
				mNextHit[prev][i] = 1;
			}
		}
	}

	void addWordcountToken(char *aToken)
	{
		int h = calcHash(aToken);
		int i;
		if (aToken == NULL || *aToken == 0)
			return;

		for (i = 0; i < mTokens; i++)
		{
			if (mHash[i] == h && strcmp(mWord[i], aToken) == 0)
			{
				tokenRef(i);
				mHits[i]++;
				mFlags[i] |= (mWordno == 0) ? 2 : (mWordno == 1 && toupper(*aToken) == *aToken) ? 1 : 0;
				return;
			}
		}

		// We don't actually care if we run out of tokens,
		// as we're looking for the most frequent N tokens
		// anyway; if a token hasn't appeared during the 
		// MAXTOKENS, it's probably not all that frequent
		// anyway.
		if (mTokens < MAXTOKENS)
		{
			mWord[mTokens] = _strdup(aToken);
			mHash[mTokens] = h;
			mHits[mTokens] = 1;
			mFlags[mTokens] = (mWordno == 0) ? 2 : (mWordno == 1 && toupper(*aToken) == *aToken) ? 1 : 0;
			mValid[mTokens] = 0;
			tokenRef(mTokens);

			mTokens++;
		}
		else
		{
			mPrevToken = -1;
		}
	}

	int valid(char *aToken)
	{
		int allupper = 1;
		int len = 0;
		while (*aToken)
		{
			if ((*aToken >= 'a' && *aToken <= 'z') ||
				(*aToken >= 'A' && *aToken <= 'Z') ||
				*aToken == '\'' ||
				*aToken == ':' ||
				*aToken == ';' ||
				*aToken == '.' ||
				*aToken == '?' ||
				*aToken == '!')
			{
				// Probably valid.
				if ((*aToken == ':' ||
					*aToken == ';' ||
					*aToken == '.' ||
					*aToken == '?' ||
					*aToken == '!'))
				{
					if (*(aToken + 1) != 0)
					{
						return 0; // not valid if not at the end of a string.
					}
					else
					{
						if (mWordno == 0)
						{
							return 0; // and not valid if the only word in the sentence
						}
						if (allupper && len > 1)
						{
							mWordno = 0; // all uppercase so we'll skip, but consider end of sentence anyway.
							return 0;
						}
						if (len < 2 && !is_alpha(aToken - 1))
						{
							return 0;
						}

						mWordno = 0; // otherwise valid, and ends the sentence
                        return 1;
                    }                
                }
                if (*aToken >= 'a' && *aToken <= 'z')
                    allupper = 0;
	        }
	        else
	        {
	            return 0;
	        }	                
	        aToken++;
	        len++;
	    }
	    if (allupper && len > 1)
	        return 0;
		if (len < 2 && !is_alpha(aToken - 1))
			return 0;
		mWordno++;
	    return 1;
	}
	
	void validate()
	{
	    int i;
	    for (i = 0; i < mTokens; i++)
	    {
	        if (mNext[i].size() > 0 || (mFlags[i] & 2) != 0)
	            mValid[i] = 1;
	    }
	}

	void wordCount(char *aString)
	{
		char temp[256];
		int p = 0;
		while (*aString)
		{
			temp[p] = *aString;
			if (*aString == '_' || 
			    *aString == '-' || 
			    *aString == '"' || 
			    *aString == ' ' || 
			    *aString == 0 || 
			    *aString == '\t' || 
			    *aString == '\r' || 
			    *aString == '\n')
			{
				temp[p] = 0;
				if (p > 0)
				{
				    if (valid(temp))
					    addWordcountToken(temp);
				}
				p = 0;
			}
			else
			{
				p++;
			}
			aString++;
		}
		temp[p] = 0;
		if (valid(temp))
		addWordcountToken(temp);
	}
};

WordCounter gWordCounter;




void read_raw_line(char *aBuffer, FILE * aFile)
{
    int i = 0, count = 0;
    int c;
    do
    {
        c = fgetc(aFile);
        if (c == '\r')
            c = fgetc(aFile);
        if (c == '\t')
            c = ' ';
        if (feof(aFile))
            c = '\n';
        aBuffer[i] = c;
        if (!feof(aFile) && c != '\n')
            i++;
		count++;
    }
    while (!feof(aFile) && c > 31 && !(count > 1000 && c == 32));
    
    // trim trailing spaces:
    while (i >= 0 && whitespace(aBuffer[i])) i--;
    i++;
    
    // terminate
    aBuffer[i] = 0;
}

// Skips empty and commented lines
void read_line(char *aBuffer, FILE * aFile)
{
    do
    {
        read_raw_line(aBuffer, aFile);
    }
    while (!feof(aFile) && aBuffer[0] == '#' && aBuffer[0] > 31);
}

void token(int aTokenIndex, char *aSource, char *aDestination)
{    
    while (aTokenIndex && *aSource)
    {
        while (*aSource && !whitespace(*aSource)) aSource++;
        while (*aSource && whitespace(*aSource)) aSource++;
        aTokenIndex--;
    }
    while (*aSource && !whitespace(*aSource))
    {
        *aDestination = *aSource;
        aDestination++;
        aSource++;
    }
    *aDestination = 0;
}

int tokencmp_for_qsort(const void * a, const void * b)
{
	int idx1 = *(int*)a;
	int idx2 = *(int*)b;
	return gWordCounter.mHits[idx2] - gWordCounter.mHits[idx1];
}

int findstarter(int val)
{
    int i;
    int last = 0;
    for (i = 0; i < gWordCounter.mTokens; i++)
    {
        if ((gWordCounter.mFlags[i] & 1) != 0 && gWordCounter.mValid[i])
        {
            last = i;
            val -= gWordCounter.mHits[i];
            if (val <= 0)
                return i;
        }
    }
    return last;
}

int nextword_nexts(int w)
{
    int val = 0;
    int i;
	for (i = 0; i < (signed)gWordCounter.mNext[w].size(); i++)
    {
        if (gWordCounter.mValid[gWordCounter.mNext[w][i]])
        {
            val += gWordCounter.mNextHit[w][i];
        }
    }
    
    val = rand() % val;
    
	for (i = 0; i < (signed)gWordCounter.mNext[w].size(); i++)
    {
        if (gWordCounter.mValid[gWordCounter.mNext[w][i]])
        {
            val -= gWordCounter.mNextHit[w][i];
            if (val <= 0)
            {
                return gWordCounter.mNext[w][i];
            }
        }
    }
	return gWordCounter.mNext[w][gWordCounter.mNext[w].size() - 1];
}

prevkeys gPrevKeys;

int nextword(int w)
{
	int sum = 0;
	gPrevKeys.shift();
	gPrevKeys.a = w;
	for (auto it = gWordCounter.mLongHit[gPrevKeys].begin(); it != gWordCounter.mLongHit[gPrevKeys].end(); ++it)
	{
		sum += it->second; 
	}
	if (sum == 0)
	{
		printf("*");
		return nextword_nexts(w);
	}
	sum = rand() % sum;
	int latest;
	for (auto it = gWordCounter.mLongHit[gPrevKeys].begin(); it != gWordCounter.mLongHit[gPrevKeys].end(); ++it)
	{
		sum -= it->second;
		if (sum <= 0)
			return it->first;
		latest = it->first;
	}
	return latest;
}

int main(int parc, char**pars)
{
    if (parc < 2)
    {
        printf("Gimme a txt file\n");
        return 0;
    }
    
    FILE * f = fopen(pars[1], "r");
    char scratch[8192];
    
	if (!f)
	{
		printf("File not found\n");
		return 0;
	}

	int lines = 0;

    while (!feof(f))// && lines < 512)
    {
		lines++;
		if ((lines & 511) == 0)
		printf("%d lines, %d tokens\r", lines, gWordCounter.mTokens);
        read_line(scratch, f);
        gWordCounter.wordCount(scratch);
    }
        
    fclose(f);
    
    gWordCounter.validate();
    
    int idx[MAXTOKENS];
    int i;
    for (i = 0; i < MAXTOKENS; i++)
        idx[i] = i;
        
	qsort(idx, gWordCounter.mTokens, sizeof(int), tokencmp_for_qsort);

	printf("Most frequent words in source material:\n");
	int total = gWordCounter.mTokens;
	if (total > 25) total = 25;
    for (i = 0; i < total; i++)
    {
        printf("%d. \"%s\"\t(%d)\n", i, gWordCounter.mWord[idx[i]], gWordCounter.mHits[idx[i]]);
    }
	printf("\n");

    int s = 0, ssum = 0;
    for (i = 0; i < gWordCounter.mTokens; i++)
    {
        if ((gWordCounter.mFlags[i] & 1) != 0 && gWordCounter.mValid[i]) { s++; ssum+=gWordCounter.mHits[i]; }
    }
       
    for (i = 0; i < 50; i++)
    {
        s = findstarter(rand() % ssum);
		gPrevKeys.reset();
        
        int ps = -1;
		int max = 0;
        while ((gWordCounter.mFlags[s] & 2) == 0 && ps != s && max < 50)
        {
            printf("%s ", gWordCounter.mWord[s]);
            ps = s;
            s = nextword(s);            
			max++;
        }
        printf("%s ", gWordCounter.mWord[s]);
        printf("\n\n");
        
    }
    
    return 0;
}
